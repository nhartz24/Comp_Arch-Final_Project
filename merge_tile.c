#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define TILE_SIZE 32 // Threshold for tiling

uint32_t *aux; // Auxiliary array for merging
size_t MAX;

static inline uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile("rdtsc" : "=a"(a), "=d"(d));
    return a | ((uint64_t)d << 32);
}

// Iterative sorting for small tiles (Insertion Sort)
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

// Merge two sorted halves
void merge(uint32_t *arr, size_t l, size_t m, size_t h) {
    size_t i = l, j = m + 1, k = l;

    // Merge into the auxiliary array
    while (i <= m && j <= h) {
        if (arr[i] <= arr[j]) {
            aux[k++] = arr[i++];
        } else {
            aux[k++] = arr[j++];
        }
    }
    while (i <= m) aux[k++] = arr[i++];
    while (j <= h) aux[k++] = arr[j++];

    // Copy back to the original array
    for (i = l; i <= h; i++) {
        arr[i] = aux[i];
    }
}

// Recursive merge sort with tiling optimization
void tiled_merge_sort(uint32_t *arr, size_t l, size_t h) {
    if (h - l + 1 <= TILE_SIZE) {
        // Sort small subarray using insertion sort
        insertion_sort(arr, l, h);
        return;
    }

    size_t m = l + (h - l) / 2;

    // Recursive calls for left and right halves
    tiled_merge_sort(arr, l, m);
    tiled_merge_sort(arr, m + 1, h);

    // Merge the two sorted halves
    merge(arr, l, m, h);
}

void sort_array(uint32_t *arr, size_t size) {
    aux = malloc(size * sizeof(uint32_t)); // Allocate auxiliary array
    if (!aux) {
        perror("Failed to allocate auxiliary array");
        exit(EXIT_FAILURE);
    }

    tiled_merge_sort(arr, 0, size - 1);

    free(aux); // Free auxiliary array
}

void print_array(uint32_t *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%u ", arr[i]);
    }
    printf("\n");
}

int main() {
    size_t size = 1 << 22; // Example: 2^20 elements
    uint32_t *arr = malloc(size * sizeof(uint32_t));
    if (!arr) {
        perror("Failed to allocate array");
        exit(EXIT_FAILURE);
    }

    srand((unsigned)time(NULL));
    for (size_t i = 0; i < size; i++) {
        arr[i] = rand();
    }

    // Declare variables for timing
    uint64_t start, end, time;

    start = rdtsc();
    sort_array(arr, size);
    end = rdtsc();
    time = end - start;

    printf("Sort time: %llu cycles\n", time);

    for (size_t i = 1; i < size; i++) {
        if (arr[i - 1] > arr[i] ) {
            printf("Simd sorting failed.\n");
            free(arr);
            return 1;
        }
    }

    // Uncomment to print the array
    // print_array(arr, size);

    free(arr);
    return 0;
}