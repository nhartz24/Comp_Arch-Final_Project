#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <immintrin.h>
#include <string.h>

/* Code for SIMD vectorization of radix sort using intel AVX2 intrinsics
 * COMPILE: gcc -O3 -mavx2 -o radix_sort_simd radix_sorting_simd.c
 * RUN: ./radix_sort_simd
 */

// function for timing cpu cycles
static inline uint64_t rdtsc() {
	unsigned long a, d;
	asm volatile("rdtsc" : "=a"(a), "=d"(d));
	return a | ((uint64_t)d << 32);
}

// Avoid making changes to this function skeleton, apart from data type changes if required
// In this starter code we have used uint32_t, feel free to change it to any other data type if required
void sort_array(uint32_t *arr, size_t size) {
	// Enter your logic here
		// allocate space for array used in sorting
	uint32_t *sorting_arr = malloc(size * sizeof(uint32_t));
	if (!sorting_arr) {
        	perror("Failed to allocate memory");
        	exit(EXIT_FAILURE);
	}

	// use radix base of 256 (one byte)	
	const int RADIX = 256;	
	const int MASK = RADIX - 1; // mask for 8 bits (0xFF)

	// aligned arrays for historgram and placements of elements (buckets)
	uint32_t counts[RADIX] __attribute__((aligned(32)));
	uint32_t placements[RADIX] __attribute__((aligned(32)));

	// main sorting loop
	// sort by each byte from least to most significant (4 digits bc 4 bytes in unint32_t)
    for (int digit = 0; digit < 4; digit++) {
		
		memset(counts, 0, sizeof(counts)); // zero out the histogram for curr byte
        __m256i mask = _mm256_set1_epi32(MASK); // mask to extract one byte
        __m256i shift = _mm256_set1_epi32(digit * 8); // shift amount for curr byte	

		// count the instances of each number at the current byte
        for (size_t i = 0; i < size; i += 8) {
            // parallelized simd extraction of byte values 8 integers at a time 
            __m256i elements = _mm256_loadu_si256((__m256i *)&arr[i]); // load 8 ints
            __m256i shifted = _mm256_srlv_epi32(elements, shift); // bit shift to target byte
            __m256i byte = _mm256_and_si256(shifted, mask); // extract  byte
                    
            // serial incrementation of the histogram
            for (int j = 0; j < 8; j++) {
                counts[_mm256_extract_epi32(byte, j)]++;
            }
        }

        // convert counts from the histogram into placements
        placements[0] = 0;
        for (int i = 1; i < RADIX; i++) {
            placements[i] = placements[i - 1] + counts[i - 1];
        }

        // sort array based on histogram placements and current byte
        for (size_t i = 0; i < size; i += 8) {
            // parellelized byte extraction
            __m256i elements = _mm256_loadu_si256((__m256i *)&arr[i]); // load 8 ints
            __m256i shifted = _mm256_srlv_epi32(elements, shift); // shift to target byte
            __m256i radix = _mm256_and_si256(shifted, mask); // extract byte

            // scatter elements into buckets based on calculated placements
            for (int j = 0; j < 8; j++) {
                uint32_t digit_value = _mm256_extract_epi32(radix, j);
                sorting_arr[placements[digit_value]] = arr[i + j];
                placements[digit_value]++;
            }
        }

        // rotate pointers from the sorting array to the sorted array
        uint32_t *swap = arr;
        arr = sorting_arr;
        sorting_arr = swap;
    }
	
	// cleanup sorting array 
    free(sorting_arr); 
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <power>\n", argv[0]);
        return 1;
    }

    int power = atoi(argv[1]);
    size_t size = 1 << power;

    // allocate space for arrays for each sorting algo (simd vs vanilla)
	uint32_t *arr = malloc(size * sizeof(uint32_t));
	if (!arr) {
		perror("Failed to allocate memory");
		exit(EXIT_FAILURE);
	}

	// fill the arrays with (the same) random numbers
    srand((unsigned)time(NULL));
	for (size_t i = 0; i < size; i++) {
		arr[i] = rand();
    }

	// declare variables for timing
    uint64_t start, end, time;
    
	// SIMD radix sorting and timing
    start = rdtsc();
	// Sort the copied array
	sort_array(arr, size);
    end = rdtsc();
    time = end - start;

	// Just output power, size, and SIMD time
    printf("%d,%zu,%lu\n", power, size, time);

	// validate sorting 
    for (size_t i = 1; i < size; i++) {
        if (arr[i - 1] > arr[i]) {
			printf("Simd sorting failed.\n");
            // cleanup on failure
            free(arr);
            return 1;
        }
        // printf("%d\n", arr[i]);
    }

	// printf("done and validated\n");

	// cleanup
	free(arr);

	return 0;
}
