#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

// Utility function to find minimum of two integers
int min(int x, int y) { return (x < y) ? x : y; }

static inline uint64_t rdtsc() {
	unsigned long a, d;
	asm volatile("rdtsc" : "=a"(a), "=d"(d));
	return a | ((uint64_t)d << 32);
}

/* Function to merge the two halves arr[l..m] and arr[m+1..h] of array arr[] */
void merge(int arr[], int l, int m, int h, int *temp)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = h - m;

    // Copy data to temp arrays
    for (i = 0; i < n1; i++)
        temp[l + i] = arr[l + i];
    for (j = 0; j < n2; j++)
        temp[m + 1 + j] = arr[m + 1 + j];

    // Merge the temp arrays back into arr[l..h]
    i = l;
    j = m + 1;
    k = l;

    while (i <= m && j <= h)
    {
        if (temp[i] <= temp[j])
        {
            arr[k] = temp[i];
            i++;
        }
        else
        {
            arr[k] = temp[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of L[], if there are any
    while (i <= m)
    {
        arr[k] = temp[i];
        i++;
        k++;
    }

    // Copy the remaining elements of R[], if there are any
    while (j <= h)
    {
        arr[k] = temp[j];
        j++;
        k++;
    }
}

/* Iterative mergesort function to sort arr[l...h] */
void mergeSort(int arr[], int l, int h, int *temp)
{
    int curr_size; // For current size of subarrays to be merged
    int left_start; // For picking starting index of left subarray to be merged

    // Merge subarrays in bottom-up manner.
    for (curr_size = 1; curr_size <= h - l; curr_size = 2 * curr_size)
    {
        // Pick starting point of different subarrays of current size
        for (left_start = l; left_start <= h - 1; left_start += 2 * curr_size)
        {
            // Find ending point of left subarray. mid+1 is starting point of right
            int mid = min(left_start + curr_size - 1, h);
            int right_end = min(left_start + 2 * curr_size - 1, h);

            // Merge Subarrays arr[left_start...mid] & arr[mid+1...right_end]
            merge(arr, left_start, mid, right_end, temp);
        }
    }
}

// Avoid making changes to this function skeleton, apart from data type changes if required
void sort_array(int *arr, size_t size)
{
    // Allocate one large array for temporary storage
    int *temp = (int *)malloc(size * sizeof(int));
    if (!temp)
    {
        perror("Failed to allocate memory for temp array");
        exit(EXIT_FAILURE);
    }

    // Call mergeSort with the temporary array
    mergeSort(arr, 0, size - 1, temp);

    // Free the allocated memory for temp array
    free(temp);
}

void print_array(int *arr, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main()
{
    size_t size = 1 << 22; // Example: Allocate space for 2^21 elements (~2MB for int)
    int *arr = malloc(size * sizeof(int)); // Allocate memory for the array
    if (!arr)
    {
        perror("Failed to allocate memory for array");
        exit(EXIT_FAILURE);
    }

    srand((unsigned)time(NULL)); // Seed the random number generator
    for (size_t i = 0; i < size; i++)
        arr[i] = rand();

    // Declare variables for timing
    uint64_t start, end, time;

    start = rdtsc();
    // Sort the array
    sort_array(arr, size);
    end = rdtsc();
    time = end - start;

    printf("Sort time: %d cycles\n", time);

    // Optionally print the array to verify sorting
    // print_array(arr, size);


    for (size_t i = 1; i < size; i++) {
        if (arr[i - 1] > arr[i] ) {
            printf("Simd sorting failed.\n");
            free(arr);
            return 1;
        }
    }

    // Free the allocated memory for the array
    free(arr);

    return 0;
}