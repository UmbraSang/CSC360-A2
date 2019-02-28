#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "pthread.h"
#include "spinlock.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items
pthread_cond_t produceable=PTHREAD_COND_INITIALIZER;
pthread_cond_t consumeable=PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

int items = 0;

void* producer (void* v) {
    int i;
  for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
        pthread_mutex_lock(&mutex);
        if(items==MAX_ITEMS){
            producer_wait_count++;
            pthread_cond_wait(&produceable, &mutex);
        }        
        items++;
        histogram[items]++;
        pthread_cond_signal(&consumeable);
        pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

void* consumer (void* v) {
    int i;
  for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    pthread_mutex_lock(&mutex);
        if(items==0){
            consumer_wait_count++;
            pthread_cond_wait(&consumeable, &mutex);
        }        
        items--;
        histogram[items]++;
        pthread_cond_signal(&produceable);
        pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

int main (int argc, char** argv) {
  pthread_t t[4];
  
  // TODO: Create Threads and Join
  pthread_create(&t[1], NULL, producer, NULL);
  pthread_create(&t[2], NULL, producer, NULL);
  pthread_create(&t[3], NULL, consumer, NULL);
  pthread_create(&t[4], NULL, consumer, NULL);

  int i;
  for(i=0; i<4; i++){
     pthread_join(t[i], NULL);
  }
  //
  
  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (pthread_t) * NUM_ITERATIONS);
}
