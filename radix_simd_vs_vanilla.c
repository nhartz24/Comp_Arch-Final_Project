#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <immintrin.h>
#include <string.h>

/* Code for SIMD vectorization of radix sort using intel AVX2 intrinsics
 * COMPILE: gcc -O3 -mavx2 -o radix_sort_simd radix_simd_vs_vanilla.c
 * RUN: ./radix_sort_simd
 */

// function for timing cpu cycles
static inline uint64_t rdtsc() {
	unsigned long a, d;
	asm volatile("rdtsc" : "=a"(a), "=d"(d));
	return a | ((uint64_t)d << 32);
}

// SIMD accelerated radix sort
void sort_array(uint32_t *arr, size_t size) {

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


// vanilla radix sort
void radix_sort_vanilla(uint32_t *arr, size_t size) {

	// use radix of base-10
	const int RADIX = 10;

	// find the max value in the array so we know how many digits we will have to sort
	uint32_t max_value = arr[0];
	for (size_t i = 1; i < size; i++) {
		if (arr[i] > max_value) {
			max_value = arr[i];
		}
	}

	// allocate space for array used in sorting
    uint32_t *sorting_arr = malloc(size * sizeof(uint32_t));
    if (!sorting_arr) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
	
    // main sorting loop
    // sort by each digit from least to most significant
    for (uint32_t exponent = 1; max_value/exponent > 0; exponent *= RADIX) {
        // histogram for current digit
        int count[RADIX];
        memset(count, 0, sizeof(count));
    
        // count each instance of each number at current digit
        for (size_t i = 0; i < size; i++) {
            count[(arr[i] / exponent) % RADIX]++;
        }
    
        // convert counts into placements in histogram 
        for (int i = 1; i < RADIX; i++) {
            count[i] += count[i - 1];
        }
    
        // sort array based on histogram placements and current digit
        for (int i = size - 1; i >= 0; i--) {
            uint32_t digit = (arr[i] / exponent) % RADIX;
            count[digit]--;
            sorting_arr[count[digit]] = arr[i];
        }
    
        // copy sorted array for current digit back to origional array
        for (size_t i = 0; i < size; i++) {
            arr[i] = sorting_arr[i];
        }
    }
	
	// claenup temporary sorting array
    free(sorting_arr);
}


int main(int argc, char *argv[]) {

    // get power for data collection
    //if (argc != 2) {
    //    printf("Usage: %s <power_of_two>\n", argv[0]);
    //    printf("Example: %s 20 for testing with 2^20 elements\n", argv[0]);
    //    return 1;
    //}
    //int power = atoi(argv[1]);
    //size_t size = 1ULL << power;
	
	// initialize random unssorted array	
	size_t size = 1 << 30; // 2^30 elements (4GB given elements are unit32_t)

	// allocate space for arrays for each sorting algo (simd vs vanilla)
	uint32_t *arr_simd = malloc(size * sizeof(uint32_t));
	uint32_t *arr_vnla = malloc(size * sizeof(uint32_t));
	if (!arr_simd || !arr_vnla) {
		perror("Failed to allocate memory");
		exit(EXIT_FAILURE);
	}

	// fill the arrays with (the same) random numbers
    srand((unsigned)time(NULL));
	for (size_t i = 0; i < size; i++) {
		arr_simd[i] = rand();
		arr_vnla[i] = arr_simd[i];
    }

    // declare variables for timing
    uint64_t start, end, simd_time, vanilla_time;

    // SIMD radix sorting and timing
    start = rdtsc();
    sort_array(arr_simd, size);
    end = rdtsc();
    simd_time = end - start;

    // vanilla radix sorting and timing
    start = rdtsc();
    radix_sort_vanilla(arr_vnla, size);
    end = rdtsc();
    vanilla_time = end - start;

    // compare sorting results in cycles and speedup
    printf("SIMD sort time: %lu cycles\n", simd_time);
    printf("Vanilla sort time: %lu cycles\n", vanilla_time);
    printf("Speedup: %.2f%%\n", ((double)(vanilla_time - simd_time) / simd_time) * 100);

    // print results for csv
    //double speedup = (double)vanilla_time / simd_time;
    //printf("%d,%zu,%lu,%lu,%.2f\n", power, size, simd_time, vanilla_time, speedup);

    // validate sorting 
    for (size_t i = 1; i < size; i++) {
        if (arr_simd[i - 1] > arr_simd[i] || arr_vnla[i - 1] > arr_vnla[i]) {
            if (arr_simd[i - 1] > arr_simd[i]) {
                printf("Simd sorting failed.\n");
            } else {
                printf("Vanilla sorting failed.\n");
            }
            // cleanup on failure
            free(arr_simd);
            free(arr_vnla);
            return 1;
        }
        // printf("%d\n", arr_simd[i]);
        // printf("%d\n", arr_vnla[i]);
    }

    // cleanup
	free(arr_simd);
	free(arr_vnla);
	
	return 0;
}
