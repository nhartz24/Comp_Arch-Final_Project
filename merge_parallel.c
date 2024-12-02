#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>

uint32_t *a;
size_t MAX;


static inline uint64_t rdtsc() {
	unsigned long a, d;
	asm volatile("rdtsc" : "=a"(a), "=d"(d));
	return a | ((uint64_t)d << 32);
}

void merge(size_t l, size_t m, size_t h) {
    size_t n1 = m - l + 1, n2 = h - m;
    size_t *L = malloc(n1 * sizeof(size_t));
    size_t *R = malloc(n2 * sizeof(size_t));
    size_t i, j, k = l;
    for (i = 0; i < n1; i++) L[i] = a[i + l];
    for (i = 0; i < n2; i++) R[i] = a[i + m + 1];
    i = j = 0;
    while (i < n1 && j < n2) {
        if (L[i] < R[j]) a[k++] = L[i++];
        else a[k++] = R[j++];
    }
    while (i < n1) a[k++] = L[i++];
    while (j < n2) a[k++] = R[j++];
    free(L);
    free(R);
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
    merge(tsk->l, tsk->m ,tsk->h);
    return NULL;
}

void sort_array(uint32_t *arr, size_t size) {

    size_t num_threads = 16, N; 

    N = num_threads;

    pthread_t threads[N];
    struct tsk tsklist[N];
    struct tsk_m mlist[N];
    size_t p = MAX / N, l = 0;


    // for(int j = 1; j < num_threads; j*=2){
    //     N = j;
    //     p = MAX / N;

        for (size_t i = 0; i < N; i++, l += p) {
            tsklist[i].l = l;
            tsklist[i].h = (i == N - 1) ? MAX - 1 : l + p - 1;
        }

        for (size_t i = 0; i < N; i++)
            pthread_create(&threads[i], NULL, merge_sort_thread, &tsklist[i]);

        for (size_t i = 0; i < N; i++)
            pthread_join(threads[i], NULL);


    // }
    //implement parallel merging if needed


    for(int j = 2; j <= num_threads; j*=2){
        l = 0;
        N = num_threads/j;
        p = MAX / N;

        for (size_t i = 0; i < N; i++, l += p) {
            mlist[i].l = l;
            mlist[i].h = (i == N - 1) ? MAX - 1 : l + p - 1;
            mlist[i].m = (mlist[i].h + mlist[i].l)/2;
        }

        for (size_t i = 0; i < N; i++){
            pthread_create(&threads[i], NULL, merge_sort_m, &mlist[i]);
        }
        for (size_t i = 0; i < N; i++){
            
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
    
 // Adjust to a reasonable number of threads
    size_t size = 1 << 22;
    // size_t size = 64;
    u_int32_t *sorted_arr = malloc(size * sizeof(uint32_t)); 
    if (!sorted_arr) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    srand((unsigned)time(NULL));
    for (size_t i = 0; i < size; i++)
        sorted_arr[i] = ((rand() % 100) + 1);

    MAX = size;
    a = sorted_arr;

    // declare variables for timing
    uint64_t start, end, time;

    start = rdtsc();
    // Sort the copied array
    sort_array(sorted_arr, size);
    end = rdtsc();
    time = end - start;

    printf("Sort time: %d cycles\n", time);


    // print_array(sorted_arr, size);

    free(sorted_arr);

}