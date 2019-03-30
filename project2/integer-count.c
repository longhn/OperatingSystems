/**
 * An application that can process multiple files containing
 * various integers and compute the frequency count of the
 * distinct integers inside the files. The application makes
 * use of pthreads by letting a seperate thread process each
 * file. The application is dependent on a thread safe hash
 * table.
 * @author Efe Acer - 21602217
 * @author Yusuf Dalva - 21602867
 * @version 1.0
 */

#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pthread.h"
#include "hash.h"

#define N 100
#define K 10
#define MAX_SIZE 500

struct num_count_pair {
    int num;
    int count;
};

typedef struct num_count_pair NumCountPair;

// Global variable(s)
HashTable* ht1; // space allocated inside library
pthread_mutex_t lock;
int total_num_count = 0;

// Function(s)
void* process_file(void* file_name);
int comparator(const void* first, const void* second);

int main(int argc, char** argv) {
    if (argc < 4) {
        printf("Error: Given number of arguments is not enough.\n");
        return -1;
    }
    int num_input_files = atoi(argv[1]);
    printf("Given number of input files: %d\n", num_input_files);
    // Main thread initializes the HashTable and the mutex lock
    ht1 = hash_init(N, K);
    pthread_mutex_init(&lock, NULL);
    // Create an array of threads
    pthread_t threads[num_input_files];
    int i;
    for(i = 0; i < num_input_files; i++) {
        if (pthread_create(&threads[i], NULL, process_file, argv[i + 2]) != 0) {
            printf("Error: Thread creation failed.\n");
            return -1;
        }
    }
    // Wait for all threads to finish
    for (i = 0; i < num_input_files; i++) {
        pthread_join(threads[i], NULL);
    }
    NumCountPair pairs[total_num_count];
    int j = 0;
    for (i = 0; i < N; i++) {
        if (ht1->arr[i] != NULL) {
            Node* curr_node = ht1->arr[i];
            while (curr_node != NULL) {
                NumCountPair pair = {.num = curr_node->k, .count = (intptr_t) curr_node->v};
                pairs[j] = pair;
                j++;
                curr_node = curr_node->next;
            }
        }
    }
    // Sort the pairs using sdandard library's quick sort
    qsort((void*) pairs, total_num_count, sizeof(NumCountPair), comparator);
    // Open the output file
    char* file = argv[argc - 1];
    FILE* fp;
    if ((fp = fopen(file, "w")) == NULL) { // file does not exist
        fp = fopen(file, "w+b"); // create it
    }
    printf("Results:\n");
    for (i = 0; i < total_num_count; i++) {
        char pair_str[MAX_SIZE];
        sprintf(pair_str, "%d: %d\n", pairs[i].num, pairs[i].count);
        printf("%s", pair_str);
        fputs(pair_str, fp);
    }
    // Main thread destroys the HashTable and the mutex lock
    hash_destroy(ht1);
	fclose(fp);
    pthread_mutex_destroy(&lock);
    return 0;
}

void* process_file(void* file_name) {
    char* file = (char*) file_name;
    printf("Thread initialized to process %s.\n", file);
    FILE* fp;
    if ((fp = fopen(file, "r")) == NULL) {
        printf("Error: Failed to open %s.\n", file);
        pthread_exit(NULL);
    }
    char buffer[MAX_SIZE];
    void* curr_count = NULL;
    while (fgets(buffer, MAX_SIZE, fp)) {
        buffer[strcspn(buffer, "\n\r")] = '\0'; // remove the newline at the end
        char num_str[strlen(buffer)];
        strcpy(num_str, buffer); // get the number as a string
        int num = atoi(num_str); // convert the number string to an int
        // In case a context switch occurs between hash_get() and hash_insert()
        // for the same integer
        pthread_mutex_lock(&lock);
        if (hash_get(ht1, num, &curr_count) == -1) { // key not found
            // Initial count of the current number is 1
            hash_insert(ht1, num, (void*) 1);
            total_num_count++;
        } else { // key already exists, its value must be updated
            void* new_count = (void*) ((intptr_t) curr_count + 1);
            hash_update(ht1, num, new_count);
        }
        pthread_mutex_unlock(&lock);
    }
	fclose(fp);
    // pthread_exit(NULL); -> this function allocates a block that is not
	//                        freed at the end of the process exit
	return NULL; // so this is a better way to end pthread execution
}

int comparator(const void* first, const void* second) {
    return ((NumCountPair*) first)->num - ((NumCountPair*) second)->num;
}
