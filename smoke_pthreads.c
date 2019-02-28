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
  pthread_mutex_t mutex;
  pthread_cond_t  match;
  pthread_cond_t  paper;
  pthread_cond_t  tobacco;
  pthread_cond_t  smoke;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  agent->mutex = PTHREAD_MUTEX_INITIALIZER;
  
  agent->paper=PTHREAD_COND_INITIALIZER;
  agent->match=PTHREAD_COND_INITIALIZER;
  agent->tobacco=PTHREAD_COND_INITIALIZER;
  agent->smoke=PTHREAD_COND_INITIALIZER;
  
  return agent;
}

/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked


//
// TODO
// You will probably need to add some procedures and struct etc.
struct threadArgs {
    struct Agent* agent;
    enum Resource type;
};

struct threadArgs* createThreadArgs(struct Agent* a, enum Resource b){
    struct threadArgs* args1 = malloc (sizeof (struct threadArgs));
    args1->agent = a;
    args1->type = b;
    return args1;
}

int matchAvail = 0;
int paperAvail = 0;
int tobaccoAvail = 0;
pthread_mutex_t actorMutex  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t matchMutex  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t paperMutex  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tobaccoMutex= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t actorsWake   = PTHREAD_COND_INITIALIZER;

void* resourceType(void* prepackage){
    struct threadArgs* package = prepackage;
    struct Agent* a = package->agent;
    enum Resource type = package->type;
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
        pthread_cond_broadcast(&actorsWake);
    }
}

void smokeIt(struct Agent* a, enum Resource type){
    smoke_count[type]++;
    pthread_cond_signal(&a->smoke);
    matchAvail = 0;
    paperAvail = 0;
    tobaccoAvail = 0;
}

void* actor(void* prepackage){
    struct threadArgs* package = prepackage;
    struct Agent* a = package->agent;
    enum Resource type = package->type;
    while(1){
        pthread_mutex_lock(&actorMutex);
        pthread_cond_wait(&actorsWake, &actorMutex);
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
        pthread_mutex_unlock(&actorMutex);
    }
}

//


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
  int i;
    for (i = 0; i < NUM_ITERATIONS; i++) {
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
    pthread_create(&t[0], NULL, agent, a);
    int i;
    // for(i=1; i<=3; i++){
    //     pthread_create(&t[i], NULL, actor, createThreadArgs(a, Resource[i]));
    //     pthread_create(&t[i+3], NULL, resourceType, createThreadArgs(a, Resource[i]));
    // }
    
    pthread_create(&t[1], NULL, actor, createThreadArgs(a, MATCH));
    pthread_create(&t[2], NULL, actor, createThreadArgs(a, PAPER));
    pthread_create(&t[3], NULL, actor, createThreadArgs(a, TOBACCO));
    pthread_create(&t[4], NULL, resourceType, createThreadArgs(a, MATCH));
    pthread_create(&t[5], NULL, resourceType, createThreadArgs(a, PAPER));
    pthread_create(&t[6], NULL, resourceType, createThreadArgs(a, TOBACCO));

    
  
  for(i=0; i<7; i++){
    pthread_join(t[i], NULL);
  }

  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}