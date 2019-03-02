#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "pthread.h"

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
  agent->mutex  = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
  
  agent->paper  = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
  agent->match  = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
  agent->tobacco= (pthread_cond_t)PTHREAD_COND_INITIALIZER;
  agent->smoke  = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
  
  return agent;
}

/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};
int pivot = 0;

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

pthread_cond_t matchActor     = PTHREAD_COND_INITIALIZER;
pthread_cond_t paperActor     = PTHREAD_COND_INITIALIZER;
pthread_cond_t tobaccoActor   = PTHREAD_COND_INITIALIZER;

char* printEnum(enum Resource type){
    switch (type){
        case MATCH:
            return "match";
            break;
        case PAPER:
            return "paper";
            break;
        case TOBACCO:
            return "tobacco";
            break;
        default:
            return "PrintEnum Failed\n";
            break;
    }
}

void actorChooser(){
    switch (pivot){
        case 6:
            pthread_cond_signal(&matchActor);
            break;
        case 5:
            pthread_cond_signal(&paperActor);
            break;
        case 3:
            pthread_cond_signal(&tobaccoActor);
            break;
        default:
            break;
        }
}

void* resourceType(void* prepackage){
    struct threadArgs* package = prepackage;
    struct Agent* a = package->agent;
    enum Resource type = package->type;
    pthread_mutex_lock(&a->mutex);
    switch(type){
        case MATCH:
            while(1){
                pthread_cond_wait(&a->match, &a->mutex);
                pivot += type;
                actorChooser();
            }
            break;
        case PAPER:
            while(1){
                pthread_cond_wait(&a->paper, &a->mutex);
                pivot += type;
                actorChooser();
            }
            break;
        case TOBACCO:
            while(1){
                pthread_cond_wait(&a->tobacco, &a->mutex);
                pivot += type;
                actorChooser();
            }
            break;
        default:
            printf("Error has occured in ResourceType\n");
            break;
    }
    pthread_mutex_unlock(&a->mutex);
}


void smokeIt(struct Agent* a, enum Resource type){
    smoke_count[type]++;
    pivot=0;
    pthread_cond_signal(&a->smoke);
}

void* actor(void* prepackage){
    struct threadArgs* package = prepackage;
    struct Agent* a = package->agent;
    enum Resource type = package->type;
    pthread_mutex_lock(&a->mutex);
    switch(type){
        case MATCH:
            while(1){
                pthread_cond_wait(&matchActor, &a->mutex);
                smokeIt(a, type);
            }
            break;
        case PAPER:
            while(1){
                pthread_cond_wait(&paperActor, &a->mutex);
                smokeIt(a, type);
            }
            break;
        case TOBACCO:
            while(1){
                pthread_cond_wait(&tobaccoActor, &a->mutex);
                smokeIt(a, type);
            }
            break;
        default:
            printf("Error has occured in ResourceType\n");
            break;
    }
    pthread_mutex_unlock(&a->mutex);
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
  pthread_t t[7];
  struct Agent*  a = createAgent();

    pthread_create(&t[1], NULL, actor, createThreadArgs(a, MATCH));
    pthread_create(&t[2], NULL, actor, createThreadArgs(a, PAPER));
    pthread_create(&t[3], NULL, actor, createThreadArgs(a, TOBACCO));
    pthread_create(&t[4], NULL, resourceType, createThreadArgs(a, MATCH));
    pthread_create(&t[5], NULL, resourceType, createThreadArgs(a, PAPER));
    pthread_create(&t[6], NULL, resourceType, createThreadArgs(a, TOBACCO));
    pthread_create(&t[0], NULL, agent, a);

    pthread_join(t[0], NULL);

  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}