#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"
#include "spinlock.h"

#define MAX_ITEMS 10
#define NUM_THREADS 4
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items
uthread_mutex_t mutex;
uthread_cond_t produceable;
uthread_cond_t consumeable;

int items = 0;

void* producer (void* v) {
  int i;
  for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    uthread_mutex_lock(mutex);
        while(items==MAX_ITEMS){
            producer_wait_count++;
            uthread_cond_wait(produceable);
        }        
        items++;
        assert (items <= MAX_ITEMS);
        histogram[items]++;
        uthread_cond_signal(consumeable);
    uthread_mutex_unlock(mutex);
  }
  return NULL;
}

void* consumer (void* v) {
  int i;
  for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    uthread_mutex_lock(mutex);
        while(items==0){
            consumer_wait_count++;
            uthread_cond_wait(consumeable);
        }        
        items--;
        assert (items >= 0);
        histogram[items]++;
        uthread_cond_signal(produceable);
    uthread_mutex_unlock(mutex);
  }
  return NULL;
}

int main (int argc, char** argv) {
    mutex=uthread_mutex_create();
    produceable=uthread_cond_create(mutex);
    consumeable=uthread_cond_create(mutex);

  uthread_t t[NUM_THREADS];

  uthread_init (NUM_THREADS);
  
  // TODO: Create Threads and Join
  int i;
  for(i=0; i<NUM_THREADS; i++){
      if(i%2==0){
          t[i]= uthread_create(producer, NULL);
      }else{
          t[i]= uthread_create(consumer, NULL);
      }
  }
  
  for(i=0; i<NUM_THREADS; i++){
     uthread_join(t[i], NULL);
  }
  //
  
  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
