#include <stdio.h>
#include <pthread.h>
#include <unistd.h> // For sleep()

// Define two mutexes
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

// Thread function
void* thread_func(void* arg) {
    int id = *(int*)arg;

    while (1) {
        if (id == 1) {
            printf("Thread 1: Locking mutex1\n");
            pthread_mutex_lock(&mutex1);
            sleep(1);

            printf("Thread 1: Trying to lock mutex2\n");
            if (pthread_mutex_trylock(&mutex2) == 0) {
                printf("Thread 1: Acquired both mutexes\n");
                pthread_mutex_unlock(&mutex2);
            } else {
                printf("Thread 1: Could not lock mutex2, releasing mutex1\n");
            }
            pthread_mutex_unlock(&mutex1);
        } else {
            printf("Thread 2: Locking mutex2\n");
            pthread_mutex_lock(&mutex2);
            sleep(1);

            printf("Thread 2: Trying to lock mutex1\n");
            if (pthread_mutex_trylock(&mutex1) == 0) {
                printf("Thread 2: Acquired both mutexes\n");
                pthread_mutex_unlock(&mutex1);
            } else {
                printf("Thread 2: Could not lock mutex1, releasing mutex2\n");
            }
            pthread_mutex_unlock(&mutex2);
        }
        sleep(1); // Prevent CPU overload
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    int id1 = 1, id2 = 2;

    // Create two threads
    pthread_create(&thread1, NULL, thread_func, &id1);
    pthread_create(&thread2, NULL, thread_func, &id2);

    // Wait for threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Program finished\n");
    return 0;
}
