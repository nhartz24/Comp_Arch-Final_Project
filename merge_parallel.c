#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>


/* COMPILE: gcc -pthread merge_parallel.c -o merge_parallel
 * RUN: ./merge_parallel
 */


uint32_t *a;
size_t MAX;
uint32_t *aux; // Global auxiliary array

static inline uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile("rdtsc" : "=a"(a), "=d"(d));
    return a | ((uint64_t)d << 32);
}

void merge(size_t l, size_t m, size_t h) {
    size_t i = l, j = m + 1, k = l;

    // Merge the two halves into the auxiliary array
    while (i <= m && j <= h) {
        if (a[i] <= a[j]) {
            aux[k++] = a[i++];
        } else {
            aux[k++] = a[j++];
        }
    }
    while (i <= m) aux[k++] = a[i++];
    while (j <= h) aux[k++] = a[j++];

    // Copy back the merged elements into the original array
    for (i = l; i <= h; i++) {
        a[i] = aux[i];
    }
}

void merge_sort(size_t l, size_t h) {
    if (l < h) {
        size_t m = (l + h) / 2;
        merge_sort(l, m);
        merge_sort(m + 1, h);
        merge(l, m, h);
    }
}

struct tsk {
    size_t l, h;
};

struct tsk_m {
    size_t l, h, m;
};

void *merge_sort_thread(void *arg) {
    struct tsk *tsk = (struct tsk *)arg;
    merge_sort(tsk->l, tsk->h);
    return NULL;
}

void *merge_sort_m(void *arg) {
    struct tsk_m *tsk = (struct tsk_m *)arg;
    merge(tsk->l, tsk->m, tsk->h);
    return NULL;
}

void sort_array(uint32_t *arr, size_t size) {
    size_t num_threads = 16, N;

    N = num_threads;

    pthread_t threads[N];
    struct tsk tsklist[N];
    struct tsk_m mlist[N];
    size_t p = MAX / N, l = 0;

    for (size_t i = 0; i < N; i++, l += p) {
        tsklist[i].l = l;
        tsklist[i].h = (i == N - 1) ? MAX - 1 : l + p - 1;
    }

    for (size_t i = 0; i < N; i++)
        pthread_create(&threads[i], NULL, merge_sort_thread, &tsklist[i]);

    for (size_t i = 0; i < N; i++)
        pthread_join(threads[i], NULL);

    for (int j = 2; j <= num_threads; j *= 2) {
        l = 0;
        N = num_threads / j;
        p = MAX / N;

        for (size_t i = 0; i < N; i++, l += p) {
            mlist[i].l = l;
            mlist[i].h = (i == N - 1) ? MAX - 1 : l + p - 1;
            mlist[i].m = (mlist[i].h + mlist[i].l) / 2;
        }

        for (size_t i = 0; i < N; i++) {
            pthread_create(&threads[i], NULL, merge_sort_m, &mlist[i]);
        }
        for (size_t i = 0; i < N; i++) {
            pthread_join(threads[i], NULL);
        }
    }
}

void print_array(uint32_t *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%u ", arr[i]);
    }
    printf("\n");
}

int main() {
    size_t size = 1 << 22; // Example: Allocate space for 2^22 elements
    uint32_t *sorted_arr = malloc(size * sizeof(uint32_t));
    if (!sorted_arr) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    aux = malloc(size * sizeof(uint32_t)); // Allocate the auxiliary array
    if (!aux) {
        perror("Failed to allocate memory for auxiliary array");
        free(sorted_arr);
        exit(EXIT_FAILURE);
    }

    srand((unsigned)time(NULL));
    for (size_t i = 0; i < size; i++)
        sorted_arr[i] = ((rand() % 100) + 1);

    MAX = size;
    a = sorted_arr;

    uint64_t start, end, time;

    start = rdtsc();
    sort_array(sorted_arr, size);
    end = rdtsc();
    time = end - start;

    printf("Sort time: %llu cycles\n", time);

    for (size_t i = 1; i < size; i++) {
        if (sorted_arr[i - 1] > sorted_arr[i] ) {
            printf("Simd sorting failed.\n");
            free(sorted_arr);
            return 1;
        }
    }

    // Uncomment to print the array
    // print_array(sorted_arr, size);

    free(sorted_arr);
    free(aux); // Free the auxiliary array
    return 0;
}
