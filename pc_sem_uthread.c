#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_sem.h"

#define MAX_ITEMS 10
#define NUM_THREADS 4
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;

struct threadArgs {
  uthread_sem_t mutex;
  uthread_sem_t consumeable;
  uthread_sem_t produceable;
};

struct threadArgs* createThreadArgs(int bufferSize){
    struct threadArgs* args1 = malloc (sizeof (struct threadArgs));
    args1->mutex = uthread_sem_create(1);
    args1->consumeable = uthread_sem_create(0);
    args1->produceable = uthread_sem_create(bufferSize);
    return args1;
}

void* producer (void* v) {
  struct threadArgs* prepackage = v;
  int i;
  for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
        uthread_sem_wait(&prepackage->produceable);
        uthread_sem_wait(&prepackage->mutex);      
        items++;
        histogram[items]++;
        uthread_sem_signal(&prepackage->consumeable);
        uthread_sem_signal(&prepackage->mutex);   
  }
  return NULL;
}

void* consumer (void* v) {
  struct threadArgs* prepackage = v;
  int i;
  for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
        uthread_sem_wait(&prepackage->consumeable);
        uthread_sem_wait(&prepackage->mutex);      
        items--;
        histogram[items]++;
        uthread_sem_signal(&prepackage->produceable);
        uthread_sem_signal(&prepackage->mutex);
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];

  uthread_init (4);

  // TODO: Create Threads and Join
    struct threadArgs* params = createThreadArgs(MAX_ITEMS);

    int i;
  for(i=0; i<NUM_THREADS; i++){
      if(i%2==0){
          pthread_create(&t[i], NULL, producer, params);
      }else{
          pthread_create(&t[i], NULL, consumer, params);
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
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
