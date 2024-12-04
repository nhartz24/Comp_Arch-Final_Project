#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define TILE_SIZE 32  // Example: Choose a tile size that fits into the L1 cache

static inline uint64_t rdtsc() {
	unsigned long a, d;
	asm volatile("rdtsc" : "=a"(a), "=d"(d));
	return a | ((uint64_t)d << 32);
}

void insertion_sort(uint32_t *arr, size_t l, size_t h) {
    for (size_t i = l + 1; i <= h; i++) {
        uint32_t key = arr[i];
        size_t j = i;
        while (j > l && arr[j - 1] > key) {
            arr[j] = arr[j - 1];
            j--;
        }
        arr[j] = key;
    }
}

void merge(uint32_t *arr, size_t l, size_t m, size_t r, uint32_t *temp) {
    size_t i = l, j = m + 1, k = l;

    while (i <= m && j <= r) {
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }

    while (i <= m) temp[k++] = arr[i++];
    while (j <= r) temp[k++] = arr[j++];

    for (i = l; i <= r; i++) {
        arr[i] = temp[i];
    }
}

void tiled_merge_sort(uint32_t *arr, size_t size) {
    uint32_t *temp = malloc(size * sizeof(uint32_t));
    if (!temp) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    // Step 1: Sort tiles using insertion sort
    for (size_t i = 0; i < size; i += TILE_SIZE) {
        size_t end = (i + TILE_SIZE - 1 < size) ? i + TILE_SIZE - 1 : size - 1;
        insertion_sort(arr, i, end);
    }

    // Step 2: Merge sorted tiles
    for (size_t curr_size = TILE_SIZE; curr_size < size; curr_size *= 2) {
        for (size_t left_start = 0; left_start < size - 1; left_start += 2 * curr_size) {
            size_t mid = left_start + curr_size - 1;
            size_t right_end = (left_start + 2 * curr_size - 1 < size - 1) ? left_start + 2 * curr_size - 1 : size - 1;

            merge(arr, left_start, mid, right_end, temp);
        }
    }

    free(temp);
}

void sort_array(uint32_t *arr, size_t size) {
    tiled_merge_sort(arr, size);
}

int main()
{
    size_t size = 1 << 22; // Example: Allocate space for 2^21 elements (~2MB for int)
    int *arr = malloc(size * sizeof(int)); // Allocate memory for the array
    if (!arr)
    {
        perror("Failed to allocate memory for array");
        exit(EXIT_FAILURE);
    }

    srand((unsigned)time(NULL)); // Seed the random number generator
    for (size_t i = 0; i < size; i++)
        arr[i] = rand();

    // Declare variables for timing
    uint64_t start, end, time;

    start = rdtsc();
    // Sort the array
    sort_array(arr, size);
    end = rdtsc();
    time = end - start;

    printf("Sort time: %d cycles\n", time);

    // Optionally print the array to verify sorting
    // print_array(arr, size);


    for (size_t i = 1; i < size; i++) {
        if (arr[i - 1] > arr[i] ) {
            printf("Simd sorting failed.\n");
            free(arr);
            return 1;
        }
    }

    // Free the allocated memory for the array
    free(arr);

    return 0;
}