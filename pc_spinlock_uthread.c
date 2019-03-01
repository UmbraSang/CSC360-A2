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
spinlock_t prodLock;
spinlock_t conLock;

int items = 0;

void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    spinlock_lock(&prodLock);
        while(items==MAX_ITEMS){
            producer_wait_count++;
        }        
        items++;
        histogram[items]++;
    spinlock_unlock(&prodLock);
  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    spinlock_lock(&conLock);
        while(items==0){
            consumer_wait_count++;
        }        
        items--;
        histogram[items]++;
        spinlock_unlock(&conLock);
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[NUM_THREADS];

  uthread_init (NUM_THREADS);
  spinlock_create(&prodLock);
  spinlock_create(&conLock);
  
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
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
