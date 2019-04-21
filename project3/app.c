/**
 * CS342 Spring 2019 - Project 3
 * A multithreaded application to test the resource allocation library (ralloc)
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
#include <time.h>

#define M 3 // number of resource types
#define N 5 // number of processes (threads)
#define R_MAX 10 // maximum number of resources for each type

// Function Declerations
void *aprocess(void *p);

// Global Variables
int handling_method; // deadlock handling method
int exist[M] = {R_MAX, R_MAX, R_MAX}; // resources existing in the system
pthread_t tids[N]; // ids of created threads
int terminated[N];

int main(int argc, char** argv) {
    srand(time(0)); // set seed for the random generator
    
    int deadlocked[N]; // array indicating deadlocked processes
    for (int i = 0; i < N; i++) {
        deadlocked[i] = 0; // initialize
        terminated[i] = 0;
    }
    
    // Change the handling method here !!!
    handling_method = DEADLOCK_AVOIDANCE;
    ralloc_init(N, M, exist, handling_method);
    printf("Library initialized.\n");
    
    for (int i = 0; i < N; ++i) {
        pthread_create(&(tids[i]), NULL, &aprocess, (void*) (size_t) i);
    }
    printf("%d processes (threads) are created.\n", N);
    
    if (handling_method == DEADLOCK_AVOIDANCE) {
        for (int i = 0; i < N; i++) {
            pthread_join(tids[i], NULL);
        }
        printf("All processes terminated without any deadlocks.\n");
        ralloc_end();
        return 0;
    }
    
    while (1) {
        sleep(3); // detection period
        if (handling_method == DEADLOCK_DETECTION) {
            int dn = ralloc_detection(deadlocked);
            if (dn > 0) {
                printf("%d processes are deadlocked.\n", dn);
                printf("Deadlocked processes are:\nProcess id(s): ");
                for (int i = 0; i < N; i++) {
                    if (deadlocked[i] == 1) {
                        printf("%d ", i);
                    }
                }
                printf("\n");
            } else {
                printf("There are no deadlocked processes.\n");
            }
        }
        // If all treads are terminated, call ralloc_end and exit
        int finish = 1;
        for (int i = 0; i < N; i++) {
            if (terminated[i] == 0) {
                finish = 0;
            }
        }
        if (finish) {
            break;
        }
    }
    printf("All processes terminated without any deadlocks.\n");
    ralloc_end();
    return 0;
}


void *aprocess(void *p) {
    int pid =  *((int*) &p);
    
    int max_demand[] = {
        1 + rand() % R_MAX,
        1 + rand() % R_MAX,
        1 + rand() % R_MAX};
    ralloc_maxdemand(pid, max_demand);
    
    for (int i = 0; i < 20; i++) {
        int first_demand[] = {
            rand() % (max_demand[0] + 1),
            rand() % (max_demand[1] + 1),
            rand() % (max_demand[2] + 1)};
        ralloc_request(pid, first_demand);
        int second_demand[] = {
            rand() % (max_demand[0] - first_demand[0] + 1),
            rand() % (max_demand[1] - first_demand[1] + 1),
            rand() % (max_demand[2] - first_demand[2] + 1)};
        ralloc_request(pid, second_demand);
        ralloc_release(pid, first_demand);
        ralloc_release(pid, second_demand);
    }
    
    for (int i = 0; i < 10; i++) {
        int first_demand[] = {
            rand() % (max_demand[0] + 1),
            rand() % (max_demand[1] + 1),
            rand() % (max_demand[2] + 1)};
        ralloc_request(pid, first_demand);
        ralloc_release(pid, first_demand);
        int second_demand[] = {
            rand() % (max_demand[0] + 1),
            rand() % (max_demand[1] + 1),
            rand() % (max_demand[2] + 1)};
        ralloc_request(pid, second_demand);
        ralloc_release(pid, second_demand);
    }
    
    terminated[pid] = 1; // process successfully terminates
    // pthread_exit(NULL); // causes memory leak
    return NULL; // this is a better practice
}
