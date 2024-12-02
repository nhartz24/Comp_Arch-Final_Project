#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

static inline uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile("rdtsc" : "=a"(a), "=d"(d));
    return a | ((uint64_t)d << 32);
}

void merge(uint32_t *arr, size_t l, size_t m, size_t h, size_t *temp) {
    size_t n1 = m - l + 1, n2 = h - m;
    size_t i, j, k = l;

    // Copy data to temp arrays
    for (i = 0; i < n1; i++) temp[i + l] = arr[i + l];
    for (i = 0; i < n2; i++) temp[i + m + 1] = arr[i + m + 1];

    i = l;
    j = m + 1;

    // Merge the temp arrays back into arr[l..h]
    while (i <= m && j <= h) {
        if (temp[i] < temp[j]) arr[k++] = temp[i++];
        else arr[k++] = temp[j++];
    }

    // Copy the remaining elements of L[], if there are any
    while (i <= m) arr[k++] = temp[i++];

    // Copy the remaining elements of R[], if there are any
    while (j <= h) arr[k++] = temp[j++];
}

void merge_sort(uint32_t *arr, size_t l, size_t h, size_t *temp) {
    if (l < h) {
        size_t m = (l + h) / 2;
        merge_sort(arr, l, m, temp); // Recursively sort the left subarray
        merge_sort(arr, m + 1, h, temp); // Recursively sort the right subarray
        merge(arr, l, m, h, temp); // Merge the sorted subarrays
    }
}

// Avoid making changes to this function skeleton, apart from data type changes if required
// In this starter code we have used uint32_t, feel free to change it to any other data type if required
void sort_array(uint32_t *arr, size_t size) {
    // Allocate one large temporary array for merging
    size_t *temp = malloc(size * sizeof(size_t));
    if (!temp) {
        perror("Failed to allocate memory for temp array");
        exit(EXIT_FAILURE);
    }

    // Call mergeSort with the temporary array
    merge_sort(arr, 0, size - 1, temp);

    // Free the allocated memory for temp array
    free(temp);
}

void print_array(uint32_t *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%u ", arr[i]);
    }
    printf("\n");
}

int main() {
    size_t size = 1 << 22; // Example: Allocate space for 2^22 elements (~4MB for uint32_t)
    uint32_t *sorted_arr = malloc(size * sizeof(uint32_t)); // Allocate memory for the sorted array
    if (!sorted_arr) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    srand((unsigned)time(NULL)); // Seed the random number generator
    for (size_t i = 0; i < size; i++) {
        sorted_arr[i] = rand();
    }

    // Declare variables for timing
    uint64_t start, end, time;

    start = rdtsc();
    // Sort the copied array
    sort_array(sorted_arr, size);
    end = rdtsc();
    time = end - start;

    printf("Sort time: %llu cycles\n", time); // Use %llu for uint64_t

    // Optionally print the array to verify sorting
    // print_array(sorted_arr, size);


    
    for (size_t i = 1; i < size; i++) {
        if (sorted_arr[i - 1] > sorted_arr[i] ) {
            printf("Simd sorting failed.\n");
            free(sorted_arr);
            return 1;
        }
    }

    // Free the allocated memory for the array
    free(sorted_arr);

    return 0;
}

















// #include <stdio.h>
// #include <stdlib.h>
// #include <stdint.h>
// #include <time.h>

// static inline uint64_t rdtsc() {
// 	unsigned long a, d;
// 	asm volatile("rdtsc" : "=a"(a), "=d"(d));
// 	return a | ((uint64_t)d << 32);
// }

// void merge(uint32_t *arr, size_t l, size_t m, size_t h) {
//     size_t n1 = m - l + 1, n2 = h - m;
//     size_t *L = malloc(n1 * sizeof(size_t));
//     size_t *R = malloc(n2 * sizeof(size_t));
//     size_t i, j, k = l;
//     for (i = 0; i < n1; i++) L[i] = arr[i + l];
//     for (i = 0; i < n2; i++) R[i] = arr[i + m + 1];
//     i = j = 0;
//     while (i < n1 && j < n2) {
//         if (L[i] < R[j]) arr[k++] = L[i++];
//         else arr[k++] = R[j++];
//     }
//     while (i < n1) arr[k++] = L[i++];
//     while (j < n2) arr[k++] = R[j++];
//     free(L);
//     free(R);
// }

// void merge_sort(uint32_t *arr, size_t l, size_t h) {
//     if (l < h) {
//         size_t m = (l + h) / 2;
//         merge_sort(arr,l, m);
//         merge_sort(arr, m + 1, h);
//         merge(arr, l, m, h);
//     }
// }

// // Avoid making changes to this function skeleton, apart from data type changes if required
// // In this starter code we have used uint32_t, feel free to change it to any other data type if required
// void sort_array(uint32_t *arr, size_t size) {
//     // Enter your logic here
//     merge_sort(arr, 0, size - 1);
// }


// void print_array(uint32_t *arr, size_t size) {
//     for (size_t i = 0; i < size; i++) {
//         printf("%u ", arr[i]);
//     }
//     printf("\n");
// }

// int main() {
    
//     size_t size = 1 << 22; // Example: Allocate space for 2^28 elements (~1GB for uint32_t)
//     uint32_t *sorted_arr = malloc(size * sizeof(uint32_t)); // Allocate memory for the sorted array
// 	if (!sorted_arr) {
// 		perror("Failed to allocate memory");
//     		exit(EXIT_FAILURE);
// 	}
// 	srand((unsigned)time(NULL)); // Seed the random number generator
// 	for (size_t i = 0; i < size; i++)
// 		sorted_arr[i] = rand();


//     // declare variables for timing
//     uint64_t start, end, time;

//     start = rdtsc();
//     // Sort the copied array
//     sort_array(sorted_arr, size);
//     end = rdtsc();
//     time = end - start;

//     printf("Sort time: %d cycles\n", time);


//     // print_array(sorted_arr, size);

//     return 0;
// }