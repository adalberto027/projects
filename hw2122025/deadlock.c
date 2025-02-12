#include <stdio.h>
#include <pthread.h>
#include <unistd.h> // For sleep()

// Define two mutexes
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

// Thread 1 function
void* thread1_func(void* arg) {
    printf("Thread 1: Locking mutex1\n");
    pthread_mutex_lock(&mutex1);
    sleep(1);  // Simulate work

    printf("Thread 1: Trying to lock mutex2\n");
    pthread_mutex_lock(&mutex2);

    printf("Thread 1: Acquired both mutexes\n");
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    
    return NULL;
}

// Thread 2 function
void* thread2_func(void* arg) {
    printf("Thread 2: Locking mutex2\n");
    pthread_mutex_lock(&mutex2);
    sleep(1);  // Simulate work

    printf("Thread 2: Trying to lock mutex1\n");
    pthread_mutex_lock(&mutex1);

    printf("Thread 2: Acquired both mutexes\n");
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // Create two threads
    pthread_create(&thread1, NULL, thread1_func, NULL);
    pthread_create(&thread2, NULL, thread2_func, NULL);

    // Wait for threads to finish (which may never happen due to deadlock)
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Program finished\n");
    return 0;
}

