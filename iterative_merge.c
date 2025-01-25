#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>


/* COMPILE: gcc iterative_merge.c -o iterative_merge
 * RUN: ./iterative_merge
 */

static inline uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile("rdtsc" : "=a"(a), "=d"(d));
    return a | ((uint64_t)d << 32);
}

// Utility function to find the minimum of two integers
int min(int x, int y) { return (x < y) ? x : y; }

/* Function to merge the two halves arr[l..m] and arr[m+1..h] of array arr[] */
void merge(int arr[], int l, int m, int h) {
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = h - m;

    // Dynamically allocate memory for temporary arrays L[] and R[]
    int *L = (int *)malloc(n1 * sizeof(int));
    int *R = (int *)malloc(n2 * sizeof(int));

    if (L == NULL || R == NULL) {
        perror("Failed to allocate memory for temporary arrays");
        exit(EXIT_FAILURE);
    }

    // Copy data to temp arrays L[] and R[]
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    // Merge the temp arrays back into arr[l..h]
    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of L[], if there are any
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy the remaining elements of R[], if there are any
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    // Free dynamically allocated memory for temp arrays
    free(L);
    free(R);
}

/* Iterative merge sort function to sort arr[l...h] */
void mergeSort(int arr[], int l, int h) {
    int curr_size; // For current size of subarrays to be merged
    int left_start; // For picking starting index of left subarray to be merged

    // Merge subarrays in bottom-up manner
    for (curr_size = 1; curr_size <= h - l; curr_size = 2 * curr_size) {
        // Pick starting point of different subarrays of current size
        for (left_start = l; left_start <= h - 1; left_start += 2 * curr_size) {
            // Find the ending point of the left subarray. mid+1 is the starting point of the right
            int mid = min(left_start + curr_size - 1, h);

            int right_end = min(left_start + 2 * curr_size - 1, h);

            // Merge Subarrays arr[left_start...mid] & arr[mid+1...right_end]
            merge(arr, left_start, mid, right_end);
        }
    }
}

// Avoid making changes to this function skeleton, apart from data type changes if required
void sort_array(uint32_t *arr, size_t size) {
    // Enter your logic here
    mergeSort(arr, 0, size - 1);
}

void print_array(uint32_t *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%u ", arr[i]);
    }
    printf("\n");
}

int main() {
    size_t size = 1 << 22; // Example: Allocate space for 2^21 elements (~8MB for uint32_t)
    uint32_t *sorted_arr = malloc(size * sizeof(uint32_t)); // Allocate memory for the sorted array
    if (!sorted_arr) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    srand((unsigned)time(NULL)); // Seed the random number generator
    for (size_t i = 0; i < size; i++)
        sorted_arr[i] = rand();

    // Declare variables for timing
    uint64_t start, end, time;

    start = rdtsc();
    // Sort the array
    sort_array(sorted_arr, size);
    end = rdtsc();
    time = end - start;

    printf("Sort time: %llu cycles\n", time);

    for (size_t i = 1; i < size; i++) {
        if (sorted_arr[i - 1] > sorted_arr[i] ) {
            printf("Simd sorting failed.\n");
            free(sorted_arr);
            return 1;
        }
    }

    // Uncomment this to print the sorted array if needed
    // print_array(sorted_arr, size);

    // Free the allocated memory
    free(sorted_arr);

    return 0;
}