#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define TILE_SIZE 32      // Size of tiles for initial sorting
#define THREAD_THRESHOLD (1 << 16) // Threshold size for spawning threads

typedef struct {
    int *arr;
    int l;
    int h;
} ThreadArgs;

static inline uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile("rdtsc" : "=a"(a), "=d"(d));
    return a | ((uint64_t)d << 32);
}

// Utility function to merge sorted halves
void merge(int arr[], int l, int m, int h) {
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = h - m;

    int *L = malloc(n1 * sizeof(int));
    int *R = malloc(n2 * sizeof(int));

    if (L == NULL || R == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < n1; i++) L[i] = arr[l + i];
    for (j = 0; j < n2; j++) R[j] = arr[m + 1 + j];

    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
}

// Utility function for insertion sort
void insertion_sort(int *arr, int l, int h) {
    for (int i = l + 1; i <= h; i++) {
        int key = arr[i];
        int j = i - 1;

        while (j >= l && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

// Recursive merge sort with multithreading
void *mergeSortThreaded(void *args) {
    ThreadArgs *arg = (ThreadArgs *)args;
    int *arr = arg->arr;
    int l = arg->l;
    int h = arg->h;

    if (l < h) {
        int m = l + (h - l) / 2;

        // If the subarray size is above the threshold, spawn threads
        if (h - l + 1 > THREAD_THRESHOLD) {
            pthread_t leftThread, rightThread;
            ThreadArgs leftArgs = {arr, l, m};
            ThreadArgs rightArgs = {arr, m + 1, h};

            pthread_create(&leftThread, NULL, mergeSortThreaded, &leftArgs);
            pthread_create(&rightThread, NULL, mergeSortThreaded, &rightArgs);

            pthread_join(leftThread, NULL);
            pthread_join(rightThread, NULL);
        } else {
            // Otherwise, process recursively in the same thread
            ThreadArgs leftArgs = {arr, l, m};
            ThreadArgs rightArgs = {arr, m + 1, h};
            mergeSortThreaded(&leftArgs);
            mergeSortThreaded(&rightArgs);
        }

        // Merge the two sorted halves
        merge(arr, l, m, h);
    }
    return NULL;
}

// Tiled merge sort entry function
void tiledMergeSort(int arr[], int size) {
    // Step 1: Tile the array and sort each tile using insertion sort
    for (int i = 0; i < size; i += TILE_SIZE) {
        int l = i;
        int h = (i + TILE_SIZE - 1 < size) ? i + TILE_SIZE - 1 : size - 1;
        insertion_sort(arr, l, h);
    }

    // Step 2: Perform multithreaded merge sort on the sorted tiles
    ThreadArgs args = {arr, 0, size - 1};
    mergeSortThreaded(&args);
}

// Avoid making changes to this function skeleton, apart from data type changes if required
void sort_array(uint32_t *arr, size_t size) {
    tiledMergeSort((int *)arr, size);
}

void print_array(uint32_t *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%u ", arr[i]);
    }
    printf("\n");
}

int main() {
    size_t size = 1 << 22; // Example: Allocate space for 2^22 elements
    uint32_t *sorted_arr = malloc(size * sizeof(uint32_t));
    if (!sorted_arr) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    srand((unsigned)time(NULL));
    for (size_t i = 0; i < size; i++)
        sorted_arr[i] = rand();

    uint64_t start, end, time;
    start = rdtsc();
    sort_array(sorted_arr, size);
    end = rdtsc();
    time = end - start;

    printf("Sort time: %llu cycles\n", time);

    // Optionally print the array to verify sorting
    // print_array(sorted_arr, size);

    free(sorted_arr);
    return 0;
}
