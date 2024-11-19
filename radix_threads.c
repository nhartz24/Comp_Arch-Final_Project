#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <immintrin.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

#define NUM_THREADS 4
#define NUM_LOCKS 10
#define NUM_RESOURCES 10

// Shared resources
//int resources[NUM_RESOURCES];

// Mutexes for shared resources
pthread_mutex_t locks[NUM_LOCKS];

typedef struct {
    int *arr;
    int exp;
    int *count;
    int thread_id;
    int *output;
    int n;
} ThreadArgs;

static inline uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile("rdtsc" : "=a"(a), "=d"(d));
    return a | ((uint64_t)d << 32);
}


// Avoid making changes to this function skeleton, apart from data type changes if
//required
// In this starter code we have used uint32_t, feel free to change it to any other
//data type if required

void* threadFunction(void* arg) {
    ThreadArgs* threadArgs = (ThreadArgs*)arg; 
    //int thread_id = *(int*)arg;
    int *arr = threadArgs->arr;
    int exp = threadArgs->exp;
    int *count = threadArgs->count;
    int thread_id = threadArgs->thread_id;
    int *output = threadArgs->output;
    int n = threadArgs->n;

    //int min_idx = (n / NUM_THREADS) * thread_id;
    //int max_idx = min_idx + (n / NUM_THREADS);

    int min_idx = floor((double)n / NUM_THREADS * thread_id);
    int max_idx = floor((double)n / NUM_THREADS * (thread_id + 1));
    printf("min idx = %d; max idx = %d\n", min_idx, max_idx);

    for (int i = max_idx - 1; i >= min_idx; i--) {
        int digit = (arr[i] / exp) % 10;

	pthread_mutex_lock(&locks[digit]);
        output[count[digit] - 1] = arr[i];
        count[digit]--;
	pthread_mutex_unlock(&locks[digit]);
    }


    free(arg); // Free allocated memory for thread ID
    return NULL;
}





// Function to find the maximum value in an array
int getMax(int arr[], int n) {
    int max = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] > max)
            max = arr[i];
    }
    return max;
}

// Function to perform counting sort based on a specific digit represented by `exp`
void countingSort(int arr[], int n, int exp) {
    pthread_t threads[NUM_THREADS];

    /*// Initialize shared resources
    for (int i = 0; i < NUM_RESOURCES; i++) {
        resources[i] = 0;
    }*/

    // Initialize mutexes
    for (int i = 0; i < NUM_LOCKS; i++) {
        if (pthread_mutex_init(&locks[i], NULL) != 0) {
            fprintf(stderr, "Failed to initialize mutex %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    int *output = (int *)malloc(n * sizeof(int)); // Output array
    //int count[10] = {0}; // Initialize count array for base 10
    int *count = (int *)malloc(10 * sizeof(int));
    for (int i = 0; i < 10; i++)
	    count[i] = 0;
    // Count occurrences of each digit at the `exp` place
    for (int i = 0; i < n; i++) {
        int digit = (arr[i] / exp) % 10;
        count[digit]++;
    }

    // Update count[i] to store the position of the next digit in the output array
    for (int i = 1; i < 10; i++) {
        count[i] += count[i - 1];
    }

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
	ThreadArgs* args = malloc(sizeof(ThreadArgs));
	args->arr = arr;
	args->exp = exp;
	args->count = count;
	args->output = output;
	args->n = n;
        /*int* thread_id = malloc(sizeof(int));
        if (thread_id == NULL) {
            fprintf(stderr, "Failed to allocate memory for thread ID\n");
            exit(EXIT_FAILURE);
        }*/
        args->thread_id = i;
        if (pthread_create(&threads[i], NULL, threadFunction, args) != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destroy mutexes
    for (int i = 0; i < NUM_LOCKS; i++) {
        pthread_mutex_destroy(&locks[i]);
    }

/*
    // Build the output array
    for (int i = n - 1; i >= 0; i--) {
        int digit = (arr[i] / exp) % 10;
        output[count[digit] - 1] = arr[i];
        count[digit]--;
    }
*/
    // Copy the output array back into the original array
    for (int i = 0; i < n; i++) {
        arr[i] = output[i];
    }

    free(output); // Free allocated memory
    free(count);
}

// Function to perform Radix Sort
void radixSort(int arr[], int n) {
    int max = getMax(arr, n); // Find the maximum number to determine the number of digits

    // Perform counting sort for each digit (1's place, 10's place, 100's place, etc.)
    for (int exp = 1; max / exp > 0; exp *= 10) {
        countingSort(arr, n, exp);
    }
}

void sort_array(int *arr, size_t size) {
// Enter your logic here
        radixSort(arr, size);

}



// Vanilla radix sort
void radix_sort_vanilla(int *arr, size_t size) {
    const int RADIX = 10;
    int max_val = arr[0];
    for (size_t i = 1; i < size; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
    }

    int *output = malloc(size * sizeof(int));
    if (!output) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    for (int exp = 1; max_val / exp > 0; exp *= RADIX) {
        int count[RADIX];
	memset(count, 0, sizeof(count));


        for (size_t i = 0; i < size; i++) {
            count[(arr[i] / exp) % RADIX]++;
        }

        for (int i = 1; i < RADIX; i++) {
            count[i] += count[i - 1];
        }

        for (int i = size - 1; i >= 0; i--) {
            output[--count[(arr[i] / exp) % RADIX]] = arr[i];
        }

        for (size_t i = 0; i < size; i++) {
            arr[i] = output[i];
        }
    }

    free(output);
}

void sort_array_vanilla(int *arr, size_t size) {
    radix_sort_vanilla(arr, size);
}

int main() {
//Initialise the array

    size_t size = 1 << 22; // Example: Allocate space for 2^15 elements (~128KB for uint32_t)
    int *arr = malloc(size * sizeof(int)); // Allocate memory for the array
    int *arr_copy = malloc(size * sizeof(int)); // Copy of the array for comparison
    if (!arr || !arr_copy) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }


	srand((unsigned)time(NULL)); // Seed the random number generator

    for (size_t i = 0; i < size; i++) {
        arr[i] = rand();
        arr_copy[i] = arr[i]; // Keep a copy for vanilla sorting
    }
	uint64_t start, end, simd_time, vanilla_time;
        start = rdtsc();
    	sort_array(arr, size);
    	end = rdtsc();
    	simd_time = end - start;
	for (int i = 0; i < 20; i++) {
		printf("val = %d\n", arr[i]);
	}
    start = rdtsc();
    sort_array_vanilla(arr_copy, size);
    end = rdtsc();
    vanilla_time = end - start;

    // Compare results
    printf("\nSorting complete.\n");
    printf("SIMD sort time: %lu cycles\n", simd_time);
    printf("Vanilla sort time: %lu cycles\n", vanilla_time);
    printf("Percentage speedup: %.2f%%\n", ((double)(vanilla_time - simd_time) / vanilla_time) * 100);

    // Validate sorting correctness
    for (size_t i = 1; i < size; i++) {
        if (arr[i - 1] > arr[i] || arr_copy[i - 1] > arr_copy[i]) {
            printf("Sorting failed.\n");
            free(arr);
            free(arr_copy);
            return 1;
        }
    }
    printf("Both sorts validated successfully.\n");



	free(arr);
	free(arr_copy);
// Print the sorted array
return 0;
}

