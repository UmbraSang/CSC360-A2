#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "pthread.h"
#include "spinlock.h"

#define MAX_ITEMS 10
#define NUM_THREADS 4
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
        while(items==MAX_ITEMS){
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
        while(items==0){
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
  pthread_t t[NUM_THREADS];
  
  // TODO: Create Threads and Join
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
