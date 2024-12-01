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
void radix_sort_simd(uint32_t *arr, size_t size) {

	// allocate space for array used in sorting
	uint32_t *temp = malloc(size * sizeof(uint32_t));
	if (!temp) {
        	perror("Failed to allocate memory");
        	exit(EXIT_FAILURE);
    	}

	// use radix base of 256 (one byte) meaning there will 4 digits per 4 byte uint32_t
	const int RADIX = 256;
	const int DIGITS = 4;
	const int MASK = RADIX - 1; // mask for 8 bits (0xFF)

	// aligned arrays for historgram and placements of elements (buckets)
	uint32_t counts[RADIX] __attribute__((aligned(32)));
	uint32_t placements[RADIX] __attribute__((aligned(32)));

	// main sorting loop
	// sort by each digit from least to most significant
    	for (int digit = 0; digit < DIGITS; digit++) {

        	// count the instances of each number at the current digit
        	memset(counts, 0, sizeof(counts));
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

        	// convert counts from the histogram into placements
        	placements[0] = 0;
        	for (int i = 1; i < RADIX; i++) {
            		placements[i] = placements[i - 1] + counts[i - 1];
        	}

        	// Step 3: Scatter elements to temp array
        	for (size_t i = 0; i < size; i += 8) {
            		__m256i data = _mm256_loadu_si256((__m256i *)&arr[i]); // Load 8 integers
            		__m256i shifted = _mm256_srlv_epi32(data, shift_vec); // Shift to get the target digit
            		__m256i radix = _mm256_and_si256(shifted, mask_vec); // Extract digit
            		for (int j = 0; j < 8; j++) {
                		uint32_t digit_value = _mm256_extract_epi32(radix, j);
                		temp[placements[digit_value]++] = arr[i + j];
            		}
        	}

        	// Step 4: Swap buffers
        	uint32_t *swap = arr;
        	arr = temp;
        	temp = swap;
    	}

    	free(temp); // Free the temporary buffer

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
    	for (int exponent = 1; max_value/exponent > 0; exponent *= RADIX) {
        	
		// histogram for current digit
		int count[RADIX];
		memset(count, 0, sizeof(count));
		
		// count each instance of each number at current digit
        	for (int i = 0; i < size; i++) {
            		count[(arr[i] / exponent) % RADIX]++;
        	}
		
		// convert counts into placements in histogram 
        	for (int i = 1; i < RADIX; i++) {
            		count[i] += count[i - 1];
        	}
		
		// sort array based on histogram placements and current digit
        	for (int i = size - 1; i >= 0; i--) {
            		sorting_arr[count[(arr[i] / exponent) % RADIX]] = arr[i];
			count[(arr[i] / exponent) % RADIX]--;
        	}
		
		// copy sorted array for current digit back to origional array
        	for (int i = 0; i < size; i++) {
            		arr[i] = sorting_arr[i];
        	}
    	}
	
	// claenup temporary sorting array
    	free(sorting_arr);
}


int main() {
	
	// initialize random unssorted array	
	size_t size = 1 << 22; // 2^22 elements (4GB given elements are unit32_t)
	
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

    	// SIMD radix sorting and timing
    	uint64_t start, end, simd_time, vanilla_time;
    	start = rdtsc();
    	radix_sort_simd(arr_simd, size);
    	end = rdtsc();
    	simd_time = end - start;

    	// vanilla radix sorting and timing
    	start = rdtsc();
   	radix_sort_vanilla(arr_vnla, size);
    	end = rdtsc();
    	vanilla_time = end - start;

    	// compare sorting results in cycles and speedup
    	printf("SIMD sort time: %d cycles\n", simd_time);
   	printf("Vanilla sort time: %d cycles\n", vanilla_time);
    	printf("Speedup: %.2f%%\n", ((double)(vanilla_time - simd_time) / simd_time) * 100);

    	// validate sorting 
    	for (size_t i = 1; i < size; i++) {
        	if (arr_simd[i - 1] > arr_simd[i] || arr_vnla[i - 1] > arr_vnla[i]) {
            		printf("Sorting failed.\n");
			// cleanup on failure
            		free(arr_simd);
            		free(arr_vnla);
            		return 1;
        	}
		// printf("%d\n", arr[i]);
    	}


    	// cleanup
	free(arr_simd);
	free(arr_vnla);
	
	return 0;
}
