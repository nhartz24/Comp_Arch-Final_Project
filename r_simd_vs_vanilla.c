#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <immintrin.h> // For AVX2 intrinsics
#include <string.h>

// Function prototypes
void sort_array_simd(uint32_t *arr, size_t size);
void sort_array_vanilla(uint32_t *arr, size_t size);
void radix_sort_simd(uint32_t *arr, uint32_t *temp, size_t size);
void radix_sort_vanilla(uint32_t *arr, size_t size);

static inline uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile("rdtsc" : "=a"(a), "=d"(d));
    return a | ((uint64_t)d << 32);
}

int main() {
    // Initialize the array
    size_t size = 1 << 22; // Example: Allocate space for 2^15 elements (~128KB for uint32_t)
    uint32_t *arr = malloc(size * sizeof(uint32_t)); // Allocate memory for the array
    uint32_t *arr_copy = malloc(size * sizeof(uint32_t)); // Copy of the array for comparison
    if (!arr || !arr_copy) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    srand((unsigned)time(NULL)); // Seed the random number generator
    for (size_t i = 0; i < size; i++) {
        arr[i] = rand();
        arr_copy[i] = arr[i]; // Keep a copy for vanilla sorting
    }

    // SIMD Sorting
    uint64_t start, end, simd_time, vanilla_time;
    start = rdtsc();
    sort_array_simd(arr, size);
    end = rdtsc();
    simd_time = end - start;

    // Vanilla Sorting
    start = rdtsc();
    sort_array_vanilla(arr_copy, size);
    end = rdtsc();
    vanilla_time = end - start;

    // Compare results
    printf("\nSorting complete.\n");
    printf("SIMD sort time: %lu cycles\n", simd_time);
    printf("Vanilla sort time: %lu cycles\n", vanilla_time);
    printf("Percentage speedup: %.2f%%\n", ((double)(vanilla_time - simd_time) / simd_time) * 100);

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

    // Cleanup
    free(arr);
    free(arr_copy);

    return 0;
}

// Wrapper for SIMD sorting
void sort_array_simd(uint32_t *arr, size_t size) {
    uint32_t *temp = malloc(size * sizeof(uint32_t)); // Temporary buffer for sorting
    if (!temp) {
        perror("Failed to allocate memory for temp buffer");
        exit(EXIT_FAILURE);
    }

    radix_sort_simd(arr, temp, size);

    free(temp); // Free the temporary buffer
}

// SIMD-accelerated radix sort
void radix_sort_simd(uint32_t *arr, uint32_t *temp, size_t size) {
    const int RADIX = 256; // Base for each digit (2^8)
    const int DIGITS = 4;  // 32-bit numbers split into 4 groups of 8 bits
    const int MASK = RADIX - 1; // Mask for 8 bits (0xFF)

    uint32_t counts[RADIX] __attribute__((aligned(32))); // Histogram for counting
    uint32_t offsets[RADIX] __attribute__((aligned(32))); // Offset positions for each bucket

    for (int digit = 0; digit < DIGITS; digit++) {
        // Step 1: Counting
        memset(counts, 0, sizeof(counts)); // Clear counts
        __m256i mask_vec = _mm256_set1_epi32(MASK); // Mask for extracting bits
        __m256i shift_vec = _mm256_set1_epi32(digit * 8); // Shift amount for the current digit

        for (size_t i = 0; i < size; i += 8) {
            __m256i data = _mm256_loadu_si256((__m256i *)&arr[i]); // Load 8 integers
            __m256i shifted = _mm256_srlv_epi32(data, shift_vec); // Shift to get the target digit
            __m256i radix = _mm256_and_si256(shifted, mask_vec); // Extract digit
            for (int j = 0; j < 8; j++) {
                counts[_mm256_extract_epi32(radix, j)]++; // Increment the histogram
            }
        }

        // Step 2: Calculate offsets
        offsets[0] = 0;
        for (int i = 1; i < RADIX; i++) {
            offsets[i] = offsets[i - 1] + counts[i - 1];
        }

        // Step 3: Scatter elements to temp array
        for (size_t i = 0; i < size; i += 8) {
            __m256i data = _mm256_loadu_si256((__m256i *)&arr[i]); // Load 8 integers
            __m256i shifted = _mm256_srlv_epi32(data, shift_vec); // Shift to get the target digit
            __m256i radix = _mm256_and_si256(shifted, mask_vec); // Extract digit
            for (int j = 0; j < 8; j++) {
                uint32_t digit_value = _mm256_extract_epi32(radix, j);
                temp[offsets[digit_value]++] = arr[i + j];
            }
        }

        // Step 4: Swap buffers
        uint32_t *swap = arr;
        arr = temp;
        temp = swap;
    }
}

// Wrapper for Vanilla sorting
void sort_array_vanilla(uint32_t *arr, size_t size) {
    radix_sort_vanilla(arr, size);
}

// Vanilla radix sort
void radix_sort_vanilla(uint32_t *arr, size_t size) {
    const int RADIX = 10;
    uint32_t max_val = arr[0];
    for (size_t i = 1; i < size; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
    }

    uint32_t *output = malloc(size * sizeof(uint32_t));
    if (!output) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    for (uint32_t exp = 1; max_val / exp > 0; exp *= RADIX) {
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

