#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

//How to Compile and Run
//gcc -pthread -o master-worker master-worker.c
//Run the program with:
//./master-worker <total_items> <max_buf_size> <num_workers> <num_masters>

int item_to_produce, curr_buf_size;
int total_items, max_buf_size, num_workers, num_masters;

int *buffer;

// this code synchronization primitives
pthread_mutex_t mutex;
pthread_cond_t cond_produce, cond_consume;

void print_produced(int num, int master) {
  printf("Produced %d by master %d\n", num, master);
}

void print_consumed(int num, int worker) {
  printf("Consumed %d by worker %d\n", num, worker);
}


//produce items and place in buffer
//modify code below to synchronize correctly
void *generate_requests_loop(void *data)
{
  int thread_id = *((int *)data);

  while(1)
    {
      pthread_mutex_lock(&mutex); // this line lock buffer

      while (curr_buf_size == max_buf_size && item_to_produce < total_items) {
        pthread_cond_wait(&cond_produce, &mutex); // here the program wait if buffer full
      }

      if(item_to_produce >= total_items) {
        pthread_mutex_unlock(&mutex);
        pthread_cond_broadcast(&cond_consume); // wake up consumers
        break;
      }
 
      buffer[curr_buf_size++] = item_to_produce;
      print_produced(item_to_produce, thread_id);
      item_to_produce++;

      pthread_cond_signal(&cond_consume); // this line signal consumer
      pthread_mutex_unlock(&mutex);
    }
  return 0;
}

//write function to be run by worker threads
//ensure that the workers call the function print_consumed when they consume an item
// worker function
void *consume_requests_loop(void *data) {
  int thread_id = *((int *)data);

  while (1) {
    pthread_mutex_lock(&mutex); // this is a lock buffer

    while (curr_buf_size == 0 && item_to_produce < total_items) {
      pthread_cond_wait(&cond_consume, &mutex); // this line wait if buffer empty
    }

    if (curr_buf_size == 0 && item_to_produce >= total_items) {
      pthread_mutex_unlock(&mutex);
      break;
    }

    int item = buffer[--curr_buf_size];
    print_consumed(item, thread_id);

    pthread_cond_signal(&cond_produce); // this is a signal producer
    pthread_mutex_unlock(&mutex);
  }
  return 0;
}

int main(int argc, char *argv[])
{
  int *master_thread_id;
  pthread_t *master_thread;
  item_to_produce = 0;
  curr_buf_size = 0;

  int i;

   if (argc < 5) {
    printf("./master-worker #total_items #max_buf_size #num_workers #masters e.g. ./exe 10000 1000 4 3\n");
    exit(1);
  }
  else {
    num_masters = atoi(argv[4]);
    num_workers = atoi(argv[3]);
    total_items = atoi(argv[1]);
    max_buf_size = atoi(argv[2]);
  }

   buffer = (int *)malloc (sizeof(int) * max_buf_size);

   // init sync primitives
   pthread_mutex_init(&mutex, NULL);
   pthread_cond_init(&cond_produce, NULL);
   pthread_cond_init(&cond_consume, NULL);

   //create master producer threads
   master_thread_id = (int *)malloc(sizeof(int) * num_masters);
   master_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_masters);
  for (i = 0; i < num_masters; i++)
    master_thread_id[i] = i;

  for (i = 0; i < num_masters; i++)
    pthread_create(&master_thread[i], NULL, generate_requests_loop, (void *)&master_thread_id[i]);

  //create worker consumer threads
  // create worker threads
  pthread_t *worker_threads = (pthread_t *)malloc(sizeof(pthread_t) * num_workers);
  int *worker_thread_ids = (int *)malloc(sizeof(int) * num_workers);
  for (i = 0; i < num_workers; i++) {
    worker_thread_ids[i] = i;
    pthread_create(&worker_threads[i], NULL, consume_requests_loop, (void *)&worker_thread_ids[i]);
  }

  //wait for all threads to complete
  for (i = 0; i < num_masters; i++)
    {
      pthread_join(master_thread[i], NULL);
      printf("master %d joined\n", i);
    }

  // join worker threads
  for (i = 0; i < num_workers; i++) {
    pthread_join(worker_threads[i], NULL);
    printf("worker %d joined\n", i);
  }

  /*----Deallocating Buffers---------------------*/
  pthread_mutex_destroy(&mutex); // destroy mutex
  pthread_cond_destroy(&cond_produce); // destroy cond vars
  pthread_cond_destroy(&cond_consume);

  free(buffer);
  free(master_thread_id);
  free(master_thread);
  free(worker_threads); // free worker memory
  free(worker_thread_ids);

  return 0;
}
