#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// Global buffer variables
static char *buff;
static int N, size = 0, next_in = 0, next_out = 0, num_items;

pthread_mutex_t mutex;
pthread_cond_t not_full;
pthread_cond_t not_empty;

void *producer(void *arg);
void *consumer(void *arg);
static void *my_malloc(size_t size);
static void add_to_queue(char item);
static char remove_from_queue(void);
static void print_buffer(void);

int main(int argc, char **argv) {
    pthread_t *ptids, *ctids;
    int i, num_prods, num_cons;

    if (argc != 4) {
        printf("usage: ./prodcons n_prods_cons n_items buff_size\n");
        exit(1);
    }
    num_prods = num_cons = atoi(argv[1]);
    num_items = atoi(argv[2]);
    N = atoi(argv[3]);

    printf("%d Producers & %d Consumers, each producing %d items, buff size %d\n",
           num_prods, num_cons, num_items, N);

    ptids = (pthread_t *)my_malloc(sizeof(pthread_t) * num_prods);
    ctids = (pthread_t *)my_malloc(sizeof(pthread_t) * num_cons);
    buff = (char *)my_malloc(sizeof(char) * N);

    // Initialize synchronization primitives
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_full, NULL);
    pthread_cond_init(&not_empty, NULL);

    // Create producer threads
    for (i = 0; i < num_prods; i++) {
        pthread_create(&ptids[i], NULL, producer, NULL);
    }

    // Create consumer threads
    for (i = 0; i < num_cons; i++) {
        pthread_create(&ctids[i], NULL, consumer, NULL);
    }

    // Wait for threads to finish
    for (i = 0; i < num_prods; i++) {
        pthread_join(ptids[i], 0);
    }
    for (i = 0; i < num_cons; i++) {
        pthread_join(ctids[i], 0);
    }

    // Cleanup
    free(buff);
    free(ptids);
    free(ctids);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);
    
    exit(0);
}

// Producer function with synchronization
void *producer(void *arg) {
    for (int i = 0; i < num_items; i++) {
        pthread_mutex_lock(&mutex);

        // Wait if buffer is full
        while (size == N) {
            pthread_cond_wait(&not_full, &mutex);
        }

        add_to_queue('A' + i % 26);
        print_buffer();

        pthread_cond_signal(&not_empty); // Notify consumers
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

// Consumer function with synchronization
void *consumer(void *arg) {
    for (int i = 0; i < num_items; i++) {
        pthread_mutex_lock(&mutex);

        // Wait if buffer is empty
        while (size == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }

        remove_from_queue();
        print_buffer();

        pthread_cond_signal(&not_full); // Notify producers
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

// Add item to buffer
static void add_to_queue(char item) {
    buff[next_in] = item;
    next_in = (next_in + 1) % N;
    size += 1;
}

// Remove item from buffer
static char remove_from_queue() {
    char item = buff[next_out];
    next_out = (next_out + 1) % N;
    size -= 1;
    return item;
}

// Print buffer state
static void print_buffer() {
    printf("Buffer size %d ***********************\n", size);
    for (int i = 0; i < size; i++) {
        int index = (next_out + i) % N;
        printf("%2d:%2c ", index, buff[index]);
    }
    printf("\nNext in: %d  Next out: %d\n", next_in, next_out);
    printf("*************************************\n");
}

// Custom malloc wrapper
static void *my_malloc(size_t size) {
    void *ret = malloc(size);
    if (!ret) {
        perror("malloc error");
        exit(1);
    }
    return ret;
}
