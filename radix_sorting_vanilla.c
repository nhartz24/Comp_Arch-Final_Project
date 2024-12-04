#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

/* Code for vanilla radix sort
 * COMPILE: gcc -o radix_sort_vanilla radix_sorting_vanilla.c
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
// vanilla radix sort
void sort_array(uint32_t *arr, size_t size) {

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
    if (argc != 2) {
        printf("Usage: %s <power>\n", argv[0]);
        return 1;
    }

    int power = atoi(argv[1]);
    size_t size = 1 << power;

    // initialize random unssorted array	
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

    // Just output power, size, and vanilla time
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
