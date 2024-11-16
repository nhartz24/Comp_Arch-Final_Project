#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> 



void merge(uint32_t *arr, int mid, int left, int right){


    int i = left, j = mid + 1, k = 0;

    uint32_t *temp = (uint32_t *)malloc((right - left + 1) * sizeof(uint32_t));

    while (i <= mid && j <= right) {

        if (arr[i] <= arr[j])
            temp[k++] = arr[i++];

        else 
            temp[k++] = arr[j++];
    }

    while (i <= mid) {
        temp[k++] = arr[i++];
    }

    while (j <= right) {
        temp[k++] = arr[j++];
    }

    for (int i = left; i <= right; i++)
        arr[i] = temp[i - left];

    free(temp);
}


void merge_sort(uint32_t *arr, int left, int right) {

    if (left < right) {
        int mid = left + (right - left) / 2;

        merge_sort(arr, left, mid);
        merge_sort(arr, mid + 1, right);

        merge(arr, left, mid, right);
    }
}


// Avoid making changes to this function skeleton, apart from data type changes if required
// In this starter code we have used uint32_t, feel free to change it to any other data type if required
void sort_array(uint32_t *arr, size_t size) {
    // Enter your logic here

    merge_sort(arr, 0, size - 1);
}

int main() {
    //Initialise the array

    uint32_t *sorted_arr = malloc(size * sizeof(uint32_t)); // Allocate memory for the sorted array
    
    // Sort the copied array
    sort_array(sorted_arr, size);

    // Print the sorted array

    return 0;
}