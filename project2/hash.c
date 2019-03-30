/**
 * Implementation of the thread safe hash table interface.
 * @author Efe Acer - 21602217
 * @author Yusuf Dalva - 21602867
 * @version 1.0
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h> // intptr_t
#include "hash.h"

// remove printf debug statements at the end

int hash_code(HashTable* hp, int k) {
    return k % hp->N;
}

void print_table(HashTable* hp) {
    printf("Hash Table\n");
    int i;
    for (i = 0; i < hp->N; i++) {
        if (hp->arr[i] != NULL) {
            printf("Bucket %d\n", i);
            Node* curr_node = hp->arr[i];
            while (curr_node != NULL) {
                printf("(key: %d, value: %ld) ", curr_node->k, (intptr_t) curr_node->v);
                curr_node = curr_node->next;
            }
            printf("\n");
        }
    }
    printf("\n");
}

HashTable *hash_init(int N, int K) {
    HashTable* hp = NULL;
    // Allocate the HashTable
    if (N < MIN_N || N > MAX_N || (N % K) != 0 || (N / K) < MIN_M || (N / K) > MAX_M) {
        printf("Error: N and K are not properly set.\n");
    }
    if ((hp = (HashTable*) malloc(sizeof(HashTable))) == NULL) {
        printf("Error: Allocation failed.\n");
        return NULL;
    }
    // Allocate the array of LinkedLists
    if ((hp->arr = (Node**) malloc(sizeof(Node*) * N)) == NULL) {
        printf("Error: Allocation failed.\n");
        return NULL;
    }
    int i;
    for (i = 0; i < N; i++) {
        hp->arr[i] = NULL;
    }
    hp->N = N;
    hp->M = N / K; // total number of regions
    // Allocate the mutex lock array
    if ((hp->locks = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t) * K)) == NULL) {
        printf("Error: Allocation failed.\n");
        return NULL;
    }
    for (i = 0; i < K; i++) {
        // Initialize and store the mutex locks
        pthread_mutex_t lock;
        pthread_mutex_init(&lock, NULL);
        hp->locks[i] = lock;
    }
    printf("HashTable successfully initialized.\n");
    return hp;
}

int hash_insert(HashTable* hp, int k, void *v) {
    int idx = hash_code(hp, k); // index of insertion
    int lock_idx = idx / hp->M;
    pthread_mutex_lock(&hp->locks[lock_idx]); // lock
    Node* curr_node = hp->arr[idx]; // list of insertion
    Node* new_node;
    // Allocate the new Node
    if ((new_node = (Node*) malloc(sizeof(Node))) == NULL) {
        pthread_mutex_unlock(&hp->locks[lock_idx]); // unlock
        printf("Error: Allocation failed.\n");
        return -1;
    }
    new_node->k = k;
    new_node->v = v;
    new_node->next = NULL;
    if (curr_node == NULL) { // list is empty
        hp->arr[idx] = new_node; // new node becomes head
    } else {
        // First node has the key we are searching for
        if (curr_node->next == NULL && curr_node->k == k) {
            printf("Error: Key is already present.\n");
            pthread_mutex_unlock(&hp->locks[lock_idx]); // unlock
            return -1;
        }
        // Search for the end of the list and key
        while (curr_node->next != NULL) {
            curr_node = curr_node->next;
            if (curr_node->k == k) {
                printf("Error: Key is already present.\n");
                pthread_mutex_unlock(&hp->locks[lock_idx]); // unlock
                return -1;
            }
        }
        curr_node->next = new_node; // new node becomes tail
    }
    pthread_mutex_unlock(&hp->locks[lock_idx]); // unlock
    return 0;
}

int hash_delete(HashTable* hp, int k) {
    int idx = hash_code(hp, k); // index of deletion
    int lock_idx = idx / hp->M;
    pthread_mutex_lock(&hp->locks[lock_idx]); // lock
    Node* curr_node = hp->arr[idx]; // list of deletion
    // Search for the key
    Node* prev_node = NULL;
    while (curr_node != NULL) {
        if (curr_node->k == k) {
            if (prev_node == NULL) { // first item contains key
                hp->arr[idx] = curr_node->next;
            } else {
                prev_node->next = curr_node->next;
            }
            free(curr_node);
            pthread_mutex_unlock(&hp->locks[lock_idx]); // unlock
            return 0;
        }
        prev_node = curr_node;
        curr_node = curr_node->next;
    }
    pthread_mutex_unlock(&hp->locks[lock_idx]); // unlock
    printf("Delete: Key not found.\n");
    return -1;
}

int hash_update(HashTable* hp, int k, void* v) {
    int idx = hash_code(hp, k); // index of update
    int lock_idx = idx / hp->M;
    pthread_mutex_lock(&hp->locks[lock_idx]); // lock
    Node* curr_node = hp->arr[idx]; // list of update
    // Search for the key
    while (curr_node != NULL) {
        if (curr_node->k == k) {
            curr_node->v = v; // update the value
            pthread_mutex_unlock(&hp->locks[lock_idx]); // unlock
            return 0;
        }
        curr_node = curr_node->next;
    }
    pthread_mutex_unlock(&hp->locks[lock_idx]); // unlock
    printf("Update: Key not found.\n");
    return -1;
}

int hash_get(HashTable* hp, int k, void** vp) {
    int idx = hash_code(hp, k); // index of retrieval
    int lock_idx = idx / hp->M;
    pthread_mutex_lock(&hp->locks[lock_idx]); // lock
    Node* curr_node = hp->arr[idx]; // list of retrieval
    // Search for the key
    while (curr_node != NULL) {
        if (curr_node->k == k) {
            *vp = curr_node->v; // retrieve value
            pthread_mutex_unlock(&hp->locks[lock_idx]); // unlock
            return 0;
        }
        curr_node = curr_node->next;
    }
    pthread_mutex_unlock(&hp->locks[lock_idx]); // unlock
    printf("Get: Key not found.\n");
    return -1;
}

int hash_destroy(HashTable* hp) {
    int i;
    for (i = 0; i < hp->N; i++) {
        Node* curr_node = hp->arr[i];
        while (curr_node != NULL) {
            Node* to_free = curr_node;
            curr_node = curr_node->next;
            to_free->next = NULL; // for safety
            free(to_free); // avoid memory leak
        }
        hp->arr[i] = NULL;
    }
    free(hp->arr);
    int K = hp->N / hp->M;
    for (i = 0; i < K; i++) {
        pthread_mutex_destroy(&(hp->locks[i]));
    }
    free(hp->locks);
    free(hp);
    printf("HashTable successfully destroyed.\n");
    return 0;
}
