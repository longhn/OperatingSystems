/**
 * CS342 Spring 2019 - Project 3
 * ralloc: Library for resource allocation with deadlock management mechanism.
 * This files contains the implementation of the ralloc interface.
 * @author Yusuf Dalva - 21602867
 * @author Efe Acer - 21602217
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "ralloc.h"

typedef struct {
    int N; // number of processes
    int M; // number of resource types
    int* available; // number of resources available for each type
    int** max_demand; // maximum demand information for each process
    int** allocation; // resources alocated to each process at an instance
    int** need; // the resource need of each process at an instance
} Banker;

// Global Variables
Banker banker;
int policy;
pthread_mutex_t lock; // mutex lock to implement monitor functionality
pthread_cond_t cond; // condition variable to implement waiting queues
int* system_max; // maximum resources of the system

// Helper Functions
void set_need(int pid);
int allocate_vector(int* vec[], int to_set[], int size);
int allocate_matrix(int** matrix[], int* to_set[], int num_rows, int num_cols);
int can_allocate(int request[], int available[]);
int validate_pid(int pid);
void update_vector(int* vec[], int to_add[], int size, int op);
void update_state(int pid, int to_handle[], int op);
int is_safe(int work[], int* need[], int* allocation[]);
int is_safe_avoidance(int pid, int demand[]);
int is_safe_detection(int procarray[]);
void free_matrix(int* matrix[], int num_rows);

// Debugging Functions
void print_vector(int size, int vector[]);
void print_matrix(int num_rows, int num_cols, int* matrix[]);
void print_curr_state();
    
int ralloc_init(int p_count, int r_count, int r_exist[], int d_handling) {
    banker.N = p_count;
    banker.M = r_count;
    policy = d_handling;
    if (!allocate_vector(&system_max, r_exist, banker.M)) {
        printf("Error: Cannot alocate space for the system_max vector.\n");
        return -1;
    }
    if (!allocate_vector(&banker.available, r_exist, banker.M)) {
        printf("Error: Cannot alocate space for the available vector.\n");
        return -1;
    }
    if (!allocate_matrix(&banker.allocation, NULL, banker.N, banker.M)) {
        printf("Error: Cannot alocate space for the allocation matrix.\n");
        return -1;
    }
    if (policy == DEADLOCK_AVOIDANCE || policy == DEADLOCK_DETECTION) {
        if (!allocate_matrix(&banker.need, NULL, banker.N, banker.M)) {
            printf("Error: Cannot alocate space for the need matrix.\n");
            return -1;
        }
    } else {
        banker.need = NULL;
    }
    if (policy == DEADLOCK_AVOIDANCE) {
        if (!allocate_matrix(&banker.max_demand, NULL, banker.N, banker.M)) {
            printf("Error: Cannot alocate space for the max_demand matrix.\n");
            return -1;
        }
    } else {
        banker.max_demand = NULL;
    }
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Error: Mutex lock initialization failed.\n");
        return -1;
    }
    if (pthread_cond_init(&cond, NULL) != 0) {
        printf("Error: Condition variable initialization failed.\n");
        return -1;
    }
    return 0;
}

int ralloc_maxdemand(int pid, int r_max[]){
    pthread_mutex_lock(&lock);
    if (!validate_pid(pid)) {
        pthread_mutex_unlock(&lock);
        return -1;
    }
    if (!can_allocate(r_max, system_max)) {
        printf("Error: Maximum demand of process %d exceeds maximum system resources.\n", pid);
        pthread_mutex_unlock(&lock);
        return -1;
    }
    if (policy == DEADLOCK_AVOIDANCE) {
        int i;
        for (i = 0; i < banker.M; i++) {
            banker.max_demand[pid][i] = r_max[i];
            // Initially (Allocated) = [0 ... 0]: Need = Max - Allocated = Max
            banker.need[pid][i] = r_max[i];
        }
    }
    pthread_mutex_unlock(&lock);
    return 0;
}

int ralloc_request(int pid, int demand[]) {
    pthread_mutex_lock(&lock);
    if (!validate_pid(pid)) {
        pthread_mutex_unlock(&lock);
        return -1;
    }
    if (!can_allocate(demand, system_max)) {
        printf("Error: The request exceeds maximum system resources.\n");
        pthread_mutex_unlock(&lock);
        return -1;
    }
    if (policy == DEADLOCK_DETECTION) {
        for (int i = 0; i < banker.M; i++) {
            banker.need[pid][i] = demand[i]; // record the request as pending
        }
    }
    if (policy == DEADLOCK_NOTHING || policy == DEADLOCK_DETECTION) {
        while (!can_allocate(demand, banker.available)) {
            pthread_cond_wait(&cond, &lock); // cannot allocate resources the request must wait
        }
        update_state(pid, demand, -1);
    } else if (policy == DEADLOCK_AVOIDANCE) {
        int safe;
        while ((safe = is_safe_avoidance(pid, demand)) != 1) {
            if (safe == -1) { // error occured in the call
                pthread_mutex_unlock(&lock);
                return -1;
            }
            pthread_cond_wait(&cond, &lock);
        }
        update_state(pid, demand, -1);
    }
    pthread_mutex_unlock(&lock);
    return 0;
}

int ralloc_release(int pid, int demand[]) {
    pthread_mutex_lock(&lock);
    if (!can_allocate(demand, banker.allocation[pid])) {
        printf("Error: Process %d tries to release more resources than it owns.\n", pid);
        pthread_mutex_unlock(&lock);
        return -1;
    }
    update_state(pid, demand, 1);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&lock);
    return 0;
}

int ralloc_detection(int procarray[]) {
    pthread_mutex_lock(&lock);
    if (policy != DEADLOCK_DETECTION) {
        printf("Error: Invalid policy.\n");
        return -1;
    }
    int num_deadlocked = is_safe_detection(procarray);
    pthread_mutex_unlock(&lock);
    return num_deadlocked;
}

int ralloc_end() {
    free(banker.available);
    if (policy == DEADLOCK_AVOIDANCE) {
        free_matrix(banker.max_demand, banker.N);
    }
    free_matrix(banker.allocation, banker.N);
    if (policy == DEADLOCK_AVOIDANCE || policy == DEADLOCK_DETECTION) {
        free_matrix(banker.need, banker.N);
    }
    free(system_max);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    return 0;
}

/**
 * Computes the and sets the need matrix according to the rule:
 * Need = Max Demand - Allocation
 */
void set_need(int pid) {
    for (int i = 0; i < banker.M; i++) {
        banker.need[pid][i] = banker.max_demand[pid][i] - banker.allocation[pid][i];
    }
}

/**
 * Allocates heap space for an int array of given size.
 * @param vec The pointer to the array that will be allocated
 * @param to_set The array that contains the initial values of
 *               the newly created array. If this parameter is null
 *               all initial values are taken as zero
 * @param size The size of the array that will be allocated
 * @return 1 on success, 0 on failure
 */
int allocate_vector(int* vec[], int to_set[], int size) {
    if ((*vec = malloc(size * sizeof(int))) == NULL) {
        return 0;
    }
    for (int i = 0; i < size; i++) {
        if (to_set == NULL) {
            (*vec)[i] = 0;
        } else {
            (*vec)[i] = to_set[i];
        }
    }
    return 1;
}

/**
 * Allocates heap space for a 2D int array of given dimensions.
 * @param matrix The pointer to the 2D array that will be allocated
 * @param to_set The 2D array that contains the initial values of
 *               the newly created 2D array. If this parameter is null
 *               all initial values are taken as zero
 * @param num_rows The number of rows in the 2D array that will be allocated
 * @param num_cols The number of columns in the 2D array that will be allocated
 * @return 1 on success, 0 on failure
 */
int allocate_matrix(int** matrix[], int* to_set[], int num_rows, int num_cols) {
    if ((*matrix = malloc(num_rows * sizeof(int*))) == NULL) {
        return 0;
    }
    for (int i = 0; i < num_rows; i++) {
        if (((*matrix)[i] = malloc(num_cols * sizeof(int))) == NULL) {
            return 0;
        }
        for (int j = 0; j < num_cols; j++) {
            if (to_set == NULL) {
                (*matrix)[i][j] = 0;
            } else {
                (*matrix)[i][j] = to_set[i][j];
            }
        }
    }
    return 1;
}

/**
 * Compares two vectors request and available, returns 1 if every item in
 * request is less than or equal to every item in available.
 * @param request First vector in comparison
 * @param available Second vector in comparison
 * @return 1 if comparison evaluates true, 0 otherwise
 */
int can_allocate(int request[], int available[]) {
    for (int i = 0; i < banker.M; i++) {
        if (request[i] > available[i]) {
            return 0;
        }
    }
    return 1;
}

/**
 * Checks whether the given process id is valid or not, a process id
 * must be in the range [0, number of processes - 1]
 * @param pid Process id to be validated
 * @return 1 if the process id is valid, 0 otherwise
 */
int validate_pid(int pid) {
    if (pid < 0 || pid >= banker.N) {
        printf("Error: The process id (pid) is invalid.\n");
        return 0;
    }
    return 1;
}

/**
 * Adds or subtracts an array from another one.
 * @param vec The array that the computation will be performed on
 * @param to_add The array that will be added or subtracted
 * @param size The size of the arrays
 * @param op If op is 1 addition is performed, if op is -1 subtraction is performed
 */
void update_vector(int* vec[], int to_add[], int size, int op) {
    for (int i = 0; i < size; i++) {
        (*vec)[i] += to_add[i] * op;
    }
}

/**
 * Updates the current state of the system by either allocating new resources or
 * releasing the existing ones.
 * @param pid The id of the process that receives or releases resources
 * @param to_handle The array representing the amount of resources allocated or
 *                  deallocated
 * @param op Determines whether to allocate or release resources in the current
 *           state (-1 for allocation, 1 for release)
 */
void update_state(int pid, int to_handle[], int op) {
    update_vector(&banker.available, to_handle, banker.M, op);
    update_vector(&banker.allocation[pid], to_handle, banker.M, -op);
    if (policy == DEADLOCK_AVOIDANCE) {
        set_need(pid);
    } else if (policy == DEADLOCK_DETECTION) {
        update_vector(&banker.need[pid], to_handle, banker.M, op);
    }
}

/**
 * A method to check whether the current system state is safe (deadlock free) or not.
 * @param work The vector indicating the currently available system resources
 * @param need The matrix indicating the current need of each process
 * @param allocation The matrix indicating the current resource allocation of the processes
 * @return 1 if the current state is safe, 0 otherwise and -1 in case of an error
 */
int is_safe(int work[], int* need[], int* allocation[]) {
    int* finish = NULL;
    if (!allocate_vector(&finish, NULL, banker.N)) {
        printf("Error: Cannot alocate space for the finish vector.\n");
        return -1;
    }
    for (int i = 0; i < banker.N; i++) {
        if (can_allocate(need[i], work) && !finish[i]) {
            update_vector(&work, allocation[i], banker.M, 1);
            finish[i] = 1;
            i = -1;
        }
    }
    for (int i = 0; i < banker.N; i++) {
        if (finish[i] == 0) { // process cannot finish
            free(finish);
            return 0;
        }
    }
    free(finish);
    return 1;
}

/**
 * A method that calls is_safe to check whether the system state will be safe if a
 * request is allocated or not.
 * @param pid The id of the requesting process
 * @param demand The demand of the requesting process
 * @return 1 if the future state is safe, 0 otherwise and -1 in case of an error
 */
int is_safe_avoidance(int pid, int demand[]) {
    if (!can_allocate(demand, banker.need[pid])) { // validate demand
        printf("Error: Process requested more than the need it reported.\n");
        return -1;
    }
    int* work = NULL; // Work = Available
    if (!allocate_vector(&work, banker.available, banker.M)) {
        printf("Error: Cannot alocate space for the work vector.\n");
        return -1;
    }
    update_vector(&work, demand, banker.M, -1); // Work = Work - Demand
    int** allocation = NULL;
    if (!allocate_matrix(&allocation, banker.allocation, banker.N, banker.M)) {
        printf("Error: Cannot alocate space for the copied allocation matrix.\n");
        free(work);
        return -1;
    }
    update_vector(&allocation[pid], demand, banker.M, 1); // Allocation = Allocation + Demand
    int** need = NULL;
    if (!allocate_matrix(&need, banker.need, banker.N, banker.M)) {
        printf("Error: Cannot alocate space for the copied need matrix.\n");
        free(work);
        free_matrix(allocation, banker.N);
        return -1;
    }
    update_vector(&need[pid], demand, banker.M, -1); // Need[pid] = Need[pid] - Demand
    int safe = is_safe(work, need, allocation); // 1 for safe
    free(work);
    free_matrix(allocation, banker.N);
    free_matrix(need, banker.N);
    return safe;
}

/**
 * A method that calls check whether the current system state is safe, it computes
 * which processes are deadlocked if any and return the number of deadlocked processes.
 * @param procarray An array whose values are set the 1 for the running processes and
 *                  -1 for the deadlocked processes
 * @return num_deadlocked: Number of deadlocked processes, -1 in case of an error
 */
int is_safe_detection(int procarray[]) {
    int* work = NULL; // Work = Available
    if (!allocate_vector(&work, banker.available, banker.M)) {
        printf("Error: Cannot alocate space for the work vector.\n");
        return -1;
    }
    int** allocation = NULL; // Work = Available
    if (!allocate_matrix(&allocation, banker.allocation, banker.N, banker.M)) {
        printf("Error: Cannot alocate space for the copied allocation matrix.\n");
        free(work);
        return -1;
    }
    int** need = NULL;
    if (!allocate_matrix(&need, banker.need, banker.N, banker.M)) {
        printf("Error: Cannot alocate space for the copied need matrix.\n");
        free(work);
        free_matrix(allocation, banker.N);
        return -1;
    }
    int* finish = NULL;
    if (!allocate_vector(&finish, NULL, banker.N)) {
        printf("Error: Cannot alocate space for the finish vector.\n");
        free(work);
        free_matrix(allocation, banker.N);
        free_matrix(need, banker.N);
        return -1;
    }
    for (int i = 0; i < banker.N; i++) {
        if (can_allocate(need[i], work) && !finish[i]) {
            update_vector(&work, allocation[i], banker.M, 1);
            finish[i] = 1;
            i = -1;
        }
    }
    int num_deadlocked = 0;
    for (int i = 0; i < banker.N; i++) {
        if (finish[i] == 0) { // process cannot finish
            num_deadlocked++;
            procarray[i] = 1;
        } else {
            procarray[i] = -1;
        }
    }
    free(finish);
    free(work);
    free_matrix(allocation, banker.N);
    free_matrix(need, banker.N);
    return num_deadlocked;
}

/**
 * Frees the heap memory occupied by a 2D array.
 * @param matrix The pointer to the 2D array
 * @param num_rows Number of arrays in the 2D array
 */
void free_matrix(int* matrix[], int num_rows) {
    for (int i = 0; i < num_rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

// Rest is printing functions for debugging purposes

void print_vector(int size, int vector[]) {
    for (int i = 0; i < size; i++) {
        printf("%d ", vector[i]);
    }
    printf("\n");
}

void print_matrix(int num_rows, int num_cols, int* matrix[]) {
    for (int i = 0; i < num_rows; i++) {
        for (int j = 0; j < num_cols; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

void print_curr_state() {
    pthread_mutex_lock(&lock);
    printf("-------------- CURRENT STATE --------------\n");
    printf("POLICY = %d\n", policy);
    printf("\nN = %d\nM = %d\n", banker.N, banker.M);
    printf("\nSYSTEM RESOURCES: \n");
    print_vector(banker.M, system_max);
    printf("\nAVAILABLE VECTOR: \n");
    print_vector(banker.M, banker.available);
    printf("\nALLOCATON MATRIX: \n");
    print_matrix(banker.N, banker.M, banker.allocation);
    if (banker.max_demand != NULL) {
        printf("\nMAX DEMAND MATRIX: \n");
        print_matrix(banker.N, banker.M, banker.max_demand);
    }
    if (banker.need != NULL) {
        printf("\nNEED MATRIX: \n");
        print_matrix(banker.N, banker.M, banker.need);
    }
    printf("-------------------------------------------\n");
    pthread_mutex_unlock(&lock);
}
