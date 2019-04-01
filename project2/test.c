/**
 * A program to make experiments on a thread safe hash
 * table. The program receives variables of the experiment
 * as inputs and perform timing experiments to measure
 * hash table performance.
 * @author Efe Acer - 21602217
 * @author Yusuf Dalva - 21602867
 * @version 1.0
 */

#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> // for measuring CPU time
#include "pthread.h"
#include "hash.h"

// Global variable(s)
HashTable* ht1; // space allocated inside library
int trials_per_thread;

// Function decleration(s)
void* perform_experiment(void* expr_no);

int main(int argc, char** argv) {
    if (argc < 5) {
        printf("Error: Given number of arguments is not enough.\n");
        return -1;
    }
    int T = atoi(argv[1]); // number of threads
    int N = atoi(argv[2]); // table size
    int K = atoi(argv[3]); // number of locks
    int W = atoi(argv[4]); // number of operations
    trials_per_thread = W / T;
    // For measurements
    clock_t start;
    clock_t end;
    // Experiment starts
    start = clock();
    // Main thread initializes the HashTable
    ht1 = hash_init(N, K);
    // Create the threads
    pthread_t threads[T];
    int i;
    for (i = 0; i < T; i++) {
        if (pthread_create(&threads[i], NULL, perform_experiment, (void*) (size_t) i) != 0) {
            printf("Error: Thread creation failed.\n");
            return -1;
        }
    }
    // Wait for all threads to finish
    for (i = 0; i < T; i++) {
        pthread_join(threads[i], NULL);
    }
    // Main thread destroys the HashTable
    hash_destroy(ht1);
    // Experiment ends
    end = clock();
    // Results
    double time_elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("EXPERIMENT RESULTS:\n");
    printf("Number of threads (T) = %d\n", T);
    printf("Table size (N) = %d\n", N);
    printf("Number of locks (K) = %d\n", K);
    printf("Number of operations (W) = %d\n", W);
    printf("Time elapsed (in seconds): %f\n", time_elapsed);
    return 0;
}

void* perform_experiment(void* expr_no) {
    int no = (intptr_t) expr_no;
    int key_start = trials_per_thread * no;
    int key_end = trials_per_thread * (no + 1);
    int i;
    // Insert operation
    for (i = key_start; i < key_end; i++) {
        hash_insert(ht1, i, (void*) 35000);
    }
    // Update operation
    for (i = key_start; i < key_end; i++) {
        hash_update(ht1, i, (void*) 36000);
    }
    // Get operation
    void* vp = NULL;
    for (i = key_start; i < key_end; i++) {
        hash_get(ht1, i, &vp);
    }
    // Delete operation
    for (i = key_start; i < key_end; i++) {
        hash_delete(ht1, i);
    }
    // pthread_exit(NULL); -> this function allocates a block that is not
    //                        freed at the end of the process exit
    return NULL; // so this is a better way to end pthread execution
}
