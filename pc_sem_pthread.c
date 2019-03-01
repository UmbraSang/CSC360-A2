#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "pthread.h"
#include <semaphore.h>

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items
sem_t mutex;
sem_t full;
sem_t empty;

int items = 0;

void* producer (void* v) {
  int i;
  for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
        sem_wait(&empty);
        sem_wait(&mutex);      
        items++;
        histogram[items]++;
        sem_post(&mutex);
        sem_post(&full);
  }
  return NULL;
}

void* consumer (void* v) {
  int i;
  for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
        sem_wait(&full);
        sem_wait(&mutex);      
        items++;
        histogram[items]++;
        sem_post(&mutex);
        sem_post(&empty);
  }
  return NULL;
}

int main (int argc, char** argv) {
  pthread_t t[NUM_THREADS];

  // TODO: Create Threads and Join
    sem_init(&mutex, 0, 1);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, MAX_ITEMS);

    int i;
  for(i=0; i<NUM_THREADS; i++){
      if(i%2==0){
          pthread_create(&t[i], NULL, producer, NULL);
      }else{
          pthread_create(&t[i], NULL, consumer, NULL);
      }
  }

  for(i=0; i<NUM_THREADS; i++){
     pthread_join(t[i], NULL);
  }

  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (pthread_t) * NUM_ITERATIONS);
}
