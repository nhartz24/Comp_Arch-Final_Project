#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>
#include <stdint.h>
#include <time.h>


#define THREADS_PER_BLOCK 1024

// function to sort blocks of size <THREADS_PER_BLOCK> which will then be parallel merged with another call from the host
// this one is optimized with shared memory

__global__ void shared_bitonic_block_sort(int* arr, int n) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int tid = threadIdx.x;

    // copy into shared memory for reduced global memory accesses
    extern __shared__ int shared_arr[];
    if (index < n) {
        shared_arr[tid] = arr[index];
    }
    __syncthreads();
    // Bitonic sort in shared memory
    for (int size = 2; size <= blockDim.x; size = size << 1) {
        for (int stride = size >> 1; stride >= 1; stride = stride >> 1) {
            int partner_index = tid ^ stride; 
            if (partner_index > tid) {  
                bool descending = ((size & tid) != 0);
                // Array is supposed to be increasing at this point
                if (!descending && shared_arr[tid] > shared_arr[partner_index]) {
                    int temp = shared_arr[tid];
                    shared_arr[tid] = shared_arr[partner_index];
                    shared_arr[partner_index] = temp;
                }
                // Array is supposed to be decreasing at this point
                else if (descending && shared_arr[tid] < shared_arr[partner_index]) {
                    int temp = shared_arr[tid];
                    shared_arr[tid] = shared_arr[partner_index];
                    shared_arr[partner_index] = temp;
                }
            }
            __syncthreads();
        }
    }
    if (index < n) {
        arr[index] = shared_arr[tid];
    }
}

// a compare and swap method unique to bitonic sort that considers whether the array should be increasing or decreasing at this point
__device__ void bitonic_swap(int* arr, int cur_index, int partner_index, bool descending) {
    // Array is supposed to be increasing at this point
    if (!descending && arr[cur_index] > arr[partner_index]) {
        int temp = arr[cur_index];
        arr[cur_index] = arr[partner_index];
        arr[partner_index] = temp;
    }
    // Array is supposed to be decreasing at this point
    else if (descending && arr[cur_index] < arr[partner_index]) {
        int temp = arr[cur_index];
        arr[cur_index] = arr[partner_index];
        arr[partner_index] = temp;
    }
}

// function to sort blocks of size <THREADS_PER_BLOCK> which will then be parallel merged with another call from the host

__global__ void bitonic_block_sort(int* arr, int n) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;

    // Bitonic sort in global memory
    for (int size = 2; size <= blockDim.x; size = size << 1) {
        for (int stride = size >> 1; stride >= 1; stride = stride >> 1) {
            int partner_index = index ^ stride; 
            if (partner_index > index) {  
                bool descending = ((size & threadIdx.x) != 0);
                bitonic_swap(arr, index, partner_index, descending);
            }
            __syncthreads();
        }
    }
}

// cuda kernel to get the input range that each thread needs to look at
// based on binary search
// Adapted from: Programming Massively Parallel Processors A Hands-on Approach

__device__ int co_rank(int k, int* A, int m, int* B, int n){
    int i = k < m ? k : m; // i = min(k, m)
    int j = k - i;
    int i_low = 0 > (k - n) ? 0 : k - n;
    int j_low = 0 > (k - m) ? 0 : k - m;
    int delta;
    bool active = true;

    while (active) {
        if (i > 0 && j < n && A[i - 1] > B[j]) {
            delta = ((i - i_low + 1) >> 1);
            j_low = j;
            j = j + delta;
            i = i - delta;
        } else if (j > 0 && i < m && B[j - 1] >= A[i]) {
            delta = ((j - j_low + 1) >> 1); 
            i_low = i;
            i = i + delta;
            j = j - delta;
        } else {
            active = false;
        }
    }

    return i;
}

// cuda kernel for one thread to sequentially merge A with B and place in C
// assumes A and B chunks are already sorted by this point
// Adapted from: Programming Massively Parallel Processors A Hands-on Approach

__device__ void merge_sequential(int* A, int m, int* B, int n, int* C) {
    int i = 0;
    int j = 0;
    int k = 0;

    // merge while we have A and B elements
    while ((i < m) && (j < n)) { 
        if (A[i] <= B[j]) {
            C[k] = A[i];
            i++;
        } else {
            C[k] = B[j];
            j++;
        }
        k++;
    }
    // remaining elements in A
    if (i == m) {
        while (j < n) {
            C[k] = B[j];
            k++;
            j++;
        }
    } 
    // remaining elements in B
    else {
        while (i < m) {
            C[k] = A[i];
            k++;
            i++;
        }
    }
}

// main cuda kernel to merge subarray A (size m) with subarray B (size n)
// Adapted from the textbook: Programming Massively Parallel Processors A Hands-on Approach
__global__ void merge_basic_kernel(int* A, int m, int* B, int n, int* C) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    int elementsPerThread = ceil((float)(m + n) / (blockDim.x * gridDim.x));

    int k_curr = tid * elementsPerThread;
    int k_next = (tid + 1) * elementsPerThread;
    
    if (m + n > k_next){
        k_next = (tid + 1) * elementsPerThread;
    }

    int i_curr = co_rank(k_curr, A, m, B, n);
    int i_next = co_rank(k_next, A, m, B, n);
    
    int j_curr = k_curr - i_curr;
    int j_next = k_next - i_next;

    merge_sequential(&A[i_curr], i_next - i_curr, &B[j_curr], j_next - j_curr, &C[k_curr]);
}

// Host function to initialize the array with random numbers
void initialize_array(int32_t* array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 10000;
    }
}

// Helper function to print an array for sanity check
void print_array(int32_t* array, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}

bool is_sorted_chunk(int32_t* chunk, int size) {
    for (int i = 1; i < size; i++) {
        if (chunk[i - 1] > chunk[i]) {
            return false;
        }
    }
    return true;
}

// checker function that checks if individual sections are sorted within the original array after the block bitonic sort, for sanity check
void check_sorted_chunks(int32_t* array, int n, int chunk_size) {
    int num_chunks = n / chunk_size;
    bool all_sorted = true;

    for (int i = 0; i < num_chunks; i++) {
        int32_t* chunk = array + i * chunk_size;  // Pointer to the start of the current chunk
        if (!is_sorted_chunk(chunk, chunk_size)) {
            //printf("Chunk %d is NOT sorted\n", i);
            all_sorted = false;
        } else {
            //printf("Chunk %d is sorted\n", i);
        }
    }

    if (all_sorted) {
        printf("All chunks are individually sorted.\n");
    } else {
        printf("Some chunks are not sorted.\n");
    }
}

// Function to check if an array is sorted in non-decreasing order, for sanity check
bool is_sorted(int32_t* array, int size) {
    for (int i = 1; i < size; i++) {
        if (array[i] < array[i - 1]) {
            //printf("%d %d %d\n", i, array[i], array[i - 1]);
            return false;
        }
    }
    return true;
}

static inline uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile ("rdtsc" : "=a" (a), "=d" (d));
    return a | ((uint64_t)d<<32);
}

// Host function to perform parallel sorting and merging
void sort_array(int32_t* h_array, int n) {
    int32_t *d_array, *d_temp;
    size_t array_size = n * sizeof(int32_t);

    // Allocate memory on the device
    cudaMalloc(&d_array, array_size);
    cudaMalloc(&d_temp, array_size);

    // Copy input array to device
    cudaMemcpy(d_array, h_array, array_size, cudaMemcpyHostToDevice);

    int num_blocks = n / THREADS_PER_BLOCK;
    bitonic_block_sort<<<num_blocks, THREADS_PER_BLOCK>>>(d_array, n);
    cudaDeviceSynchronize();

    int* src = d_array;
    int* dst = d_temp;
    int chunk_size = THREADS_PER_BLOCK;
    int num_chunks = n / chunk_size;

    while (num_chunks > 1) {
        int num_merges = num_chunks / 2;

        // Launch merging kernels for the current stage
        for (int i = 0; i < num_merges; i++) {
            // these are pointers to the first, second, and merged arrays that we will send to the device
            int* A_chunk = src + i * 2 * chunk_size;
            int* B_chunk = A_chunk + chunk_size;
            int* C_chunk = dst + i * 2 * chunk_size;

            // Calculate number of threads and blocks for this merge
            int num_elements = 2 * chunk_size;
            int num_threads_per_block = THREADS_PER_BLOCK;
            int num_blocks = (num_elements + num_threads_per_block - 1) / num_threads_per_block;
            //printf("CALLING KERNEL WITH %d %d %d\n", num_blocks, num_threads_per_block, num_threads_per_block * sizeof(int));
            merge_basic_kernel<<<num_blocks, num_threads_per_block>>>(A_chunk, chunk_size, B_chunk, chunk_size, C_chunk);
        }

        // Wait for all merges in this stage to complete
        cudaDeviceSynchronize();

        // Swap src and dst for the next merge stage
        int* temp = src;
        src = dst;
        dst = temp;

        // Double the chunk size for the next stage
        chunk_size *= 2;
        num_chunks = num_chunks / 2;  // Round up to account for odd number of chunks
    }

    // Copy the final sorted array back to host
    cudaMemcpy(h_array, src, array_size, cudaMemcpyDeviceToHost);

    // Free device memory
    cudaFree(d_array);
    cudaFree(d_temp);
}

int main() {
    uint64_t start , end, start1, end1;
    int n = 1 << 30; 
    int32_t *sorted_array = (int32_t*)malloc(n * sizeof(int32_t));

    initialize_array(sorted_array, n);

    printf("Original Array:\n");
    print_array(sorted_array, 16); // Print first 16 elements

    // start clock
    start = rdtsc();

    start1 = clock();

    // sort array
    sort_array(sorted_array, n);

    // end clock
    end1 = clock();
    end = rdtsc();


    printf("The GPU mergesort took this many ticks: %llu \n", end - start);
    printf("The GPU mergesort took this many seconds: %f \n", (double)(end1 - start1) / CLOCKS_PER_SEC);

    printf("Sorted Array:\n");
    print_array(sorted_array, 16); // Print first 16 elements

    // Verify if the array is sorted
    if (is_sorted(sorted_array, n)) {
        //printf("The array is sorted correctly.\n");
    } else {
        printf("The array is NOT sorted correctly.\n");
    }

    free(sorted_array);
    return 0;
}
