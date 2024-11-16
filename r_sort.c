#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <immintrin.h> // For AVX2 intrinsics
#include <string.h>

// Function prototypes
void sort_array(uint32_t *arr, size_t size);
void radix_sort_simd(uint32_t *arr, uint32_t *temp, size_t size);

static inline uint64_t rdtsc() {
	 unsigned long a, d;
	asm volatile ("rdtsc" : "=a" (a), "=d" (d));
	return a | ((uint64_t)d<<32);
}


// Main function
int main() {
    // Initialize the array
    size_t size = 1 << 5; // Example: Allocate space for 2^20 elements (~4MB for uint32_t)
    uint32_t *sorted_arr = malloc(size * sizeof(uint32_t)); // Allocate memory for the sorted array
    if (!sorted_arr) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    srand((unsigned)time(NULL)); // Seed the random number generator
    for (size_t i = 0; i < size; i++) {
        sorted_arr[i] = rand();
	printf("%d ", sorted_arr[i]);
    }

    // Sort the array
    uint64_t start, end, speed;
    start = rdtsc(); // timery
    sort_array(sorted_arr, size);
    end = rdtsc(); // time
    speed = end - start;
    printf("\nspeed: %d\n", speed);
     

    printf("Sorting complete.\n");
    for (size_t i = 0; i < size; i++) {
    	printf("%d ", sorted_arr[i]);
	if (i > 2 && sorted_arr[i - 1] > sorted_arr[i]) {
		printf("fail");
		return 0;
	}
    }

    // Cleanup
    free(sorted_arr);

    return 0;
}

// Sort an array using SIMD-accelerated radix sort
void sort_array(uint32_t *arr, size_t size) {
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
