#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_THREADS 4
#define BUCKET_COUNT 10
#define ARRAY_SIZE 100

// Bucket structure
typedef struct {
    int *array;
    int count;
    int size;
} Bucket;

// Thread argument structure
typedef struct {
    int *array;
    int size;
    int thread_id;
} ThreadArg;

// Global array of buckets
Bucket buckets[BUCKET_COUNT];

// Function to initialize buckets
void init_buckets() {
    for (int i = 0; i < BUCKET_COUNT; i++) {
        buckets[i].array = (int *)malloc(ARRAY_SIZE * sizeof(int));
        buckets[i].count = 0;
        buckets[i].size = ARRAY_SIZE;
    }
}

// Function to distribute elements into buckets
void *distribute_elements(void *arg) {
    ThreadArg *targ = (ThreadArg *)arg;
    int start = (targ->size / NUM_THREADS) * targ->thread_id;
    int end = start + (targ->size / NUM_THREADS);

    for (int i = start; i < end; i++) {
        int index = targ->array[i] / (ARRAY_SIZE / BUCKET_COUNT);
        if (buckets[index].count >= buckets[index].size) {
            // Resize bucket if needed
            buckets[index].size *= 2;
            buckets[index].array = (int *)realloc(buckets[index].array, buckets[index].size * sizeof(int));
        }
        buckets[index].array[buckets[index].count++] = targ->array[i];
    }

    return NULL;
}

// Function to sort buckets
void *sort_buckets(void *arg) {
    int index = *(int *)arg;
    qsort(buckets[index].array, buckets[index].count, sizeof(int), compare);
    return NULL;
}

// Comparison function for qsort
int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

// Function to merge buckets back into the original array
void merge_buckets(int *array, int size) {
    int index = 0;
    for (int i = 0; i < BUCKET_COUNT; i++) {
        for (int j = 0; j < buckets[i].count; j++) {
            array[index++] = buckets[i].array[j];
        }
    }
}

// Main function to demonstrate parallel bucket sort using pthreads
int main() {
    int array[ARRAY_SIZE];
    pthread_t threads[NUM_THREADS];
    ThreadArg thread_args[NUM_THREADS];

    // Initialize and fill the array with random numbers
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = rand() % (ARRAY_SIZE * BUCKET_COUNT);
    }

    // Initialize buckets
    init_buckets();

    // Distribute elements into buckets using multiple threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].array = array;
        thread_args[i].size = ARRAY_SIZE;
        thread_args[i].thread_id = i;
        pthread_create(&threads[i], NULL, distribute_elements, &thread_args[i]);
    }

    // Wait for distribution to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Sort each bucket using separate threads
    for (int i = 0; i < BUCKET_COUNT; i++) {
        pthread_create(&threads[i], NULL, sort_buckets, &i);
    }

    // Wait for sorting to finish
    for (int i = 0; i < BUCKET_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    // Merge sorted buckets back into the original array
    merge_buckets(array, ARRAY_SIZE);

    // Print sorted array
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

    // Free memory allocated for buckets
    for (int i = 0; i < BUCKET_COUNT; i++) {
        free(buckets[i].array);
    }

    return 0;
}
