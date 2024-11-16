#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

// Avoid making changes to this function skeleton, apart from data type changes if required
// In this starter code we have used uint32_t, feel free to change it to any other data type if required
void sort_array(uint32_t *arr, size_t size) {
	// Enter your logic here
}

int main() {
    	//Initialise the array
	size_t size = 1 << 28; // Example: Allocate space for 2^28 elements (~1GB for uint32_t)
   	uint32_t *sorted_arr = malloc(size * sizeof(uint32_t)); // Allocate memory for the sorted array
	if (!sorted_arr) {
		perror("Failed to allocate memory");
    		exit(EXIT_FAILURE);
	}
	srand((unsigned)time(NULL)); // Seed the random number generator
	for (size_t i = 0; i < size; i++) {
		sorted_arr[i] = rand();
		// printf("%d, ", sorted_arr[i]);
	}
    
    	// Sort the copied array
    	sort_array(sorted_arr, size);

    	// Print the sorted array
	printf("done\n");

    	return 0;
}
