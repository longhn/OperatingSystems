#ifndef HASH_H
#define HASH_H

#include <pthread.h>

#define MIN_N 100
#define MAX_N 1000
#define MIN_M 10
#define MAX_M 1000

struct node {
    int k; // key
    void* v; // value
    struct node* next; // reference to the next node
};

typedef struct node Node;

struct hash_table {
    int N; // total size
    int M; // total number of mutex locks
    struct node** arr; // array of linked lists
    pthread_mutex_t* locks; // array of mutex locks
};

typedef struct hash_table HashTable; 

HashTable *hash_init(int N, int K);
int hash_insert(HashTable *hp, int k, void* v);
int hash_delete(HashTable *hp, int k);
int hash_update(HashTable *hp, int, void *v);
int hash_get(HashTable *hp, int k, void **vp);
int hash_destroy(HashTable *hp);

// Debug purposes
void print_table(HashTable* hp);

#endif /* HASH_H */
