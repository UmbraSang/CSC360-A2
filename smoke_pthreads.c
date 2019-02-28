#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "pthread.h"
#include "uthread_mutex_cond.h"

#define NUM_ITERATIONS 1000

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

struct Agent {
  pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t  match=PTHREAD_COND_INITIALIZER;
  pthread_cond_t  paper=PTHREAD_COND_INITIALIZER;
  pthread_cond_t  tobacco=PTHREAD_COND_INITIALIZER;
  pthread_cond_t  smoke=PTHREAD_COND_INITIALIZER;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  /*
  agent->mutex = PTHREAD_MUTEX_INITIALIZER;
  
  pthread_cond_init(&agent->paper, NULL);
  pthread_cond_init(&agent->match, NULL);
  pthread_cond_init(&agent->tobacco, NULL);
  pthread_cond_init(&agent->smoke, NULL);
  */
  return agent;
}

//
// TODO
// You will probably need to add some procedures and struct etc.
int matchAvail = 0;
int paperAvail = 0;
int tobaccoAvail = 0;
pthread_mutex_t actorMutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t matchMutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t paperMutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tobaccoMutex=PTHREAD_MUTEX_INITIALIZER;

void* resourceType(Resource type){
    while(1){
        switch (type){
            case MATCH:
                pthread_cond_wait(&a->match, &matchMutex);
                matchAvail=1;
                break;
            case PAPER:
                pthread_cond_wait(&a->paper, &paperMutex);
                paperAvail=1;
                break;
            case TOBACCO:
                pthread_cond_wait(&a->tobacco, &tobaccoMutex);
                tobaccoAvail=1;
                break;
            default:
                printf("Error has occured in ResourceType\n");
                break;
        }
        pthread_cond_broadcast(&actorMutex);
    }
}

void* actor(Resource type){
    while(1){
        pthread_mutex_lock(&a->actorMutex);
        switch (type){
            case MATCH:
                if(paperAvail && tobaccoAvail){
                    smokeIt(type);
                }
                break;
            case PAPER:
                if(matchAvail && tobaccoAvail){
                    smokeIt(type);
                }
                break;
            case TOBACCO:
                if(paperAvail && matchAvail){
                    smokeIt(type);
                }
                break;
            default:
                printf("Error has occured in Actor\n");
                break;
        }
        pthread_mutex_unlock(&a->actorMutex);
    }
}

void smokeIt(Resource type){
    smoke_count[type]++;
    pthread_cond_signal(&a->smoke);
    matchAvail = 0;
    paperAvail = 0;
    tobaccoAvail = 0;
}

//

/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked

/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can re-write it if you like, but be sure that all it does
 * is choose 2 random reasources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agent (void* av) {
  struct Agent* a = av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};
  
  pthread_mutex_lock (&a->mutex);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
      int r = random() % 3;
      signal_count [matching_smoker [r]] ++;
      int c = choices [r];
      if (c & MATCH) {
        VERBOSE_PRINT ("match available\n");
        pthread_cond_signal (&a->match);
      }
      if (c & PAPER) {
        VERBOSE_PRINT ("paper available\n");
        pthread_cond_signal (&a->paper);
      }
      if (c & TOBACCO) {
        VERBOSE_PRINT ("tobacco available\n");
        pthread_cond_signal (&a->tobacco);
      }
      VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
      pthread_cond_wait (&a->smoke, &a->mutex);
    }
  pthread_mutex_unlock (&a->mutex);
  return NULL;
}

int main (int argc, char** argv) {
  //uthread_init (7);
  pthread_t t[7];
  struct Agent*  a = createAgent();
  // TODO
    int i;
    for(i=0; i<3; i++){
        pthread_create(&t[i], NULL, actor, Resource[i]);
        pthread_create(&t[i+4], NULL, resourceType, Resource[i]);
    }
    /*
    pthread_create(&t[1], NULL, actor, MATCH);
    pthread_create(&t[2], NULL, actor, PAPER);
    pthread_create(&t[3], NULL, actor, TOBACCO);
    */

  pthread_join (pthread_create(&t[1], NULL, agent, a);, 0);
  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}