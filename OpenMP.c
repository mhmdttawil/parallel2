#include <omp.h>
#include <stdlib.h>
#include <stdio.h>

#define BUCKET_COUNT 10
#define ARRAY_SIZE 100

// Bucket structure
typedef struct {
    int *array;
    int count;
    int size;
} Bucket;

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
void distribute_elements(int *array, int size) {
    #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        int index = array[i] / (ARRAY_SIZE / BUCKET_COUNT);
        #pragma omp critical
        {
            if (buckets[index].count >= buckets[index].size) {
                // Resize bucket if needed
                buckets[index].size *= 2;
                buckets[index].array = (int *)realloc(buckets[index].array, buckets[index].size * sizeof(int));
            }
            buckets[index].array[buckets[index].count++] = array[i];
        }
    }
}

// Function to sort buckets
void sort_buckets() {
    #pragma omp parallel for
    for (int i = 0; i < BUCKET_COUNT; i++) {
        qsort(buckets[i].array, buckets[i].count, sizeof(int), compare);
    }
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

// Main function to demonstrate parallel bucket sort using OpenMP
int main() {
    int array[ARRAY_SIZE];

    // Initialize and fill the array with random numbers
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = rand() % (ARRAY_SIZE * BUCKET_COUNT);
    }

    // Initialize buckets
    init_buckets();

    // Distribute elements into buckets using OpenMP
    distribute_elements(array, ARRAY_SIZE);

    // Sort each bucket using OpenMP
    sort_buckets();

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
