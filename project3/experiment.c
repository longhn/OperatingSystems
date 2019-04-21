/**
 * CS342 Spring 2019 - Project 3
 * A multithreaded program to make experiments using the resource allocation library (ralloc)
 * @author Yusuf Dalva - 21602867
 * @author Efe Acer - 21602217
 */

#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pthread.h"
#include "ralloc.h"
#include <time.h> // for CPU time measurements

#define M 3 // number of resource types
#define N 5 // number of processes (threads)

// Function Declerations
void *aprocess(void *p);
void *bprocess(void *p);

// Global Variables
int handling_method; // deadlock handling method
int exist[M] = {10, 5, 7}; // resources existing in the system
pthread_t tids[N]; // ids of created threads

int main(int argc, char** argv) {
    // Change the handling method here !!!
    handling_method = DEADLOCK_DETECTION;
    ralloc_init(N, M, exist, handling_method);
    printf ("Library initialized.\n");
    
    clock_t start = clock();
    
    for (int i = 0; i < N; ++i) {
        pthread_create(&(tids[i]), NULL, &aprocess, (void*) (size_t) i);
    }
    printf("%d processes (threads) are created.\n", N);
    
    for (int i = 0; i < N; i++) {
        pthread_join(tids[i], NULL);
    }
    printf("Initial state is established.\n");
    
    for (int i = 0; i < N; ++i) {
        pthread_create(&(tids[i]), NULL, &bprocess, (void*) (size_t) i);
    }
    
    for (int i = 0; i < N; i++) {
        pthread_join(tids[i], NULL);
    }
    printf("All processes terminated without any deadlocks.\n");
    
    clock_t end = clock();
    
    ralloc_end();
    
    double time_elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("Time elapsed (in seconds): %f\n", time_elapsed);
    
    return 0;
}


void *aprocess(void *p) {
    int pid =  *((int*) &p);
    
    int max_demand[M];
    int request[M];
    
    if (pid == 0) {
        max_demand[0] = 7;
        max_demand[1] = 5;
        max_demand[2] = 3;
        request[0] = 0;
        request[1] = 1;
        request[2] = 0;
    } else if (pid == 1) {
        max_demand[0] = 3;
        max_demand[1] = 2;
        max_demand[2] = 2;
        request[0] = 2;
        request[1] = 0;
        request[2] = 0;
    } else if (pid == 2) {
        max_demand[0] = 9;
        max_demand[1] = 0;
        max_demand[2] = 2;
        request[0] = 3;
        request[1] = 0;
        request[2] = 2;
    } else if (pid == 3) {
        max_demand[0] = 2;
        max_demand[1] = 2;
        max_demand[2] = 2;
        request[0] = 2;
        request[1] = 1;
        request[2] = 1;
    } else if (pid == 4) {
        max_demand[0] = 4;
        max_demand[1] = 3;
        max_demand[2] = 3;
        request[0] = 0;
        request[1] = 0;
        request[2] = 2;
    }
    
    ralloc_maxdemand(pid, max_demand);
    ralloc_request(pid, request);
    
    // pthread_exit(NULL); // causes memory leak
    return NULL; // this is a better practice
}

void *bprocess(void *p) {
    int pid =  *((int*) &p);
    
    int max_demand[M];
    int request[M];
    
    if (pid == 0) {
        max_demand[0] = 7;
        max_demand[1] = 5;
        max_demand[2] = 3;
        request[0] = 7;
        request[1] = 4;
        request[2] = 3;
    } else if (pid == 1) {
        max_demand[0] = 3;
        max_demand[1] = 2;
        max_demand[2] = 2;
        request[0] = 1;
        request[1] = 2;
        request[2] = 2;
    } else if (pid == 2) {
        max_demand[0] = 9;
        max_demand[1] = 0;
        max_demand[2] = 2;
        request[0] = 6;
        request[1] = 0;
        request[2] = 0;
    } else if (pid == 3) {
        max_demand[0] = 2;
        max_demand[1] = 2;
        max_demand[2] = 2;
        request[0] = 0;
        request[1] = 1;
        request[2] = 1;
    } else if (pid == 4) {
        max_demand[0] = 4;
        max_demand[1] = 3;
        max_demand[2] = 3;
        request[0] = 4;
        request[1] = 3;
        request[2] = 1;
    }
    
    if (handling_method == DEADLOCK_DETECTION) {
        int procarray[N] = {0, 0, 0, 0, 0};
        ralloc_detection(procarray);
    }
    
    ralloc_request(pid, request);
    ralloc_release(pid, max_demand);
    
    // pthread_exit(NULL); // causes memory leak
    return NULL; // this is a better practice
}

