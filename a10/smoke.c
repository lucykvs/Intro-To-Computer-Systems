#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "uthreads/uthread.h"
#include "uthreads/uthread_mutex_cond.h"

#define NUM_ITERATIONS 1000

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__)
#else
#define VERBOSE_PRINT(S, ...) ((void) 0) // do nothing
#endif

struct Agent* a;
struct Helper* helper;
struct Smoker * st;
struct Smoker * sp;
struct Smoker * sm;

uthread_t agent_thread, st_thread, sp_thread, sm_thread, hm_thread, hp_thread, ht_thread;

struct Agent {
  uthread_mutex_t mutex;
  uthread_cond_t  match;
  uthread_cond_t  paper;
  uthread_cond_t  tobacco;
  uthread_cond_t  smoke;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  agent->mutex   = uthread_mutex_create();
  agent->paper   = uthread_cond_create(agent->mutex);
  agent->match   = uthread_cond_create(agent->mutex);
  agent->tobacco = uthread_cond_create(agent->mutex);
  agent->smoke   = uthread_cond_create(agent->mutex);
  return agent;
}

struct Helper {
  struct Agent* agent;
  uthread_cond_t tobaccoPaper;
  uthread_cond_t tobaccoMatch;
  uthread_cond_t paperMatch;
  uthread_cond_t smoke;
  uthread_cond_t helperSmoke;
  int match, paper, tobacco;

};

struct Helper* createHelper(struct Agent* agent) {
  struct Helper* helper = malloc(sizeof(struct Helper));
  helper->agent = agent;
  helper->tobaccoPaper = uthread_cond_create(agent->mutex);
  helper->tobaccoMatch = uthread_cond_create(agent->mutex);
  helper->paperMatch   = uthread_cond_create(agent->mutex);
  helper->helperSmoke  = uthread_cond_create(agent->mutex);
  helper->match        = 0;
  helper->paper        = 0;
  helper->tobacco      = 0;
  return helper;

}

void * helperMatch(struct Helper* helper) {
  // struct Helper* helper = h;
  uthread_mutex_lock(helper->agent->mutex);
  printf("helperMatch is running...\n");
  while(1) {
    uthread_cond_wait(helper->agent->match);
    printf("match signal recieved...\n");
    helper->match = 1;
    printf("match: %d, paper: %d, tobacco: %d\n", helper->match, helper->paper, helper->tobacco);
    if(helper->match && helper->paper) {
      uthread_cond_signal(helper->paperMatch);
      uthread_cond_wait(helper->helperSmoke);
      helper->paper = 0;
      helper->match = 0;
      helper->tobacco = 0;
      uthread_cond_signal(helper->agent->smoke);
    } else if (helper->match && helper->tobacco) {
      uthread_cond_signal(helper->tobaccoMatch);
      uthread_cond_wait(helper->helperSmoke);
      helper->paper = 0;
      helper->match = 0;
      helper->tobacco = 0;
      uthread_cond_signal(helper->agent->smoke);
    }
  }
  uthread_mutex_unlock(helper->agent->mutex);
  return NULL;
}

void * helperPaper(struct Helper* helper) {
  // struct Helper* helper = h;
  uthread_mutex_lock(helper->agent->mutex);
  printf("helperPaper is running...\n");
  while(1) {
    uthread_cond_wait(helper->agent->paper);
    printf("paper signal recieved...\n");
    helper->paper = 1;
    printf("match: %d, paper: %d, tobacco: %d\n", helper->match, helper->paper, helper->tobacco);
    if(helper->paper && helper->match) {
      uthread_cond_signal(helper->paperMatch);
      uthread_cond_wait(helper->helperSmoke);
      helper->paper = 0;
      helper->match = 0;
      helper->tobacco = 0;
      uthread_cond_signal(helper->agent->smoke);
    } else if (helper->paper && helper->tobacco) {
      uthread_cond_signal(helper->tobaccoPaper);
      uthread_cond_wait(helper->helperSmoke);
      helper->paper = 0;
      helper->match = 0;
      helper->tobacco = 0;
      uthread_cond_signal(helper->agent->smoke);
    }

  }
  uthread_mutex_unlock(helper->agent->mutex);
  return NULL;
}

void * helpertobacco(struct Helper *helper) {
  // struct Helper* helper = h;
  uthread_mutex_lock(helper->agent->mutex);
  printf("helpertobacco is running...\n");
  while(1) {
    uthread_cond_wait(helper->agent->tobacco);
    printf("tobacco signal recieved...\n");
    helper->tobacco = 1;
    printf("match: %d, paper: %d, tobacco: %d\n", helper->match, helper->paper, helper->tobacco);
    if(helper->tobacco && helper->paper) {
      uthread_cond_signal(helper->tobaccoPaper);
      uthread_cond_wait(helper->helperSmoke);
      helper->paper = 0;
      helper->match = 0;
      helper->tobacco = 0;
      uthread_cond_signal(helper->agent->smoke);
    } else if (helper->tobacco && helper->match) {
      uthread_cond_signal(helper->tobaccoMatch);
      uthread_cond_wait(helper->helperSmoke);
      helper->paper = 0;
      helper->match = 0;
      helper->tobacco = 0;
      uthread_cond_signal(helper->agent->smoke);
    }
  }
  uthread_mutex_unlock(helper->agent->mutex);
  return NULL;
}

// # of threads waiting for a signal. Used to ensure that the agent
// only signals once all other threads are ready.
int num_active_threads = 0;

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked
//
// TODO
// You will probably need to add some procedures and struct etc.
//

/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};

struct Smoker {
  uthread_mutex_t mutex;
  uthread_cond_t resourcePair;
  uthread_cond_t smoke;
  uthread_cond_t firstSmoke;
};
 
struct Smoker* createSmoker(struct Agent * agent, struct Helper* helper, enum Resource r) {
  struct Smoker* smoker = malloc(sizeof (struct Smoker));
  smoker->mutex = agent->mutex;
  smoker->smoke = helper->helperSmoke;
  smoker->firstSmoke = agent->smoke;
  switch(r) {
    case MATCH:
      smoker->resourcePair = helper->tobaccoPaper;
      break;
    case PAPER:
      smoker->resourcePair = helper->tobaccoMatch;
      break;
    case TOBACCO:
      smoker->resourcePair = helper->paperMatch;
      break;
  };
  return smoker;
}

void * tobaccoSmoke(void* s){
  struct Smoker* smoker = s;
  uthread_mutex_lock(smoker->mutex);
  printf("tobaccoSmoke is running...\n");
  num_active_threads++;
  printf("num_active_threads is now %d\n", num_active_threads);
  uthread_cond_signal(smoker->firstSmoke);
  while(1) {
    uthread_cond_wait(smoker->resourcePair);
    uthread_cond_signal(smoker->smoke);
    smoke_count[TOBACCO]++;
  }
  uthread_mutex_unlock(smoker->mutex);
  return NULL;
}

void * paperSmoke(void* s){
  struct Smoker* smoker = s;
  uthread_mutex_lock(smoker->mutex);
  printf("paperSmoke is running...\n");
  num_active_threads++;
  printf("num_active_threads is now %d\n", num_active_threads);
  uthread_cond_signal(smoker->firstSmoke);
  while(1) {
    uthread_cond_wait(smoker->resourcePair);
    uthread_cond_signal(smoker->smoke);
    smoke_count[PAPER]++;
  }
  uthread_mutex_unlock(smoker->mutex);
  return NULL;
}

void * matchSmoke(void* s){
  struct Smoker* smoker = s;
  uthread_mutex_lock(smoker->mutex);
  printf("matchSmoke is running...\n");
  num_active_threads++;
  printf("num_active_threads is now %d\n", num_active_threads);
  uthread_cond_signal(smoker->firstSmoke);
  while(1) {
    uthread_cond_wait(smoker->resourcePair);
    uthread_cond_signal(smoker->smoke);
    smoke_count[MATCH]++;  
  }
  uthread_mutex_unlock(smoker->mutex);
  return NULL;
}



/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can modify it if you like, but be sure that all it does
 * is choose 2 random resources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agent (void* av) {
  printf("agent is running...");
  struct Agent* a = av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};

  srandom(time(NULL));
  
  uthread_mutex_lock (a->mutex);
  // Wait until all other threads are waiting for a signal
  while (num_active_threads < 3) {
    printf("agent is waiting...\n");
    uthread_cond_wait (a->smoke);
  }

  printf("agent is done waiting...\n");

  

  for (int i = 0; i < NUM_ITERATIONS; i++) {
    int r = random() % 6;
    printf("%d\n", r);
    switch(r) {
    case 0:
      signal_count[TOBACCO]++;
      VERBOSE_PRINT ("match available\n");
      uthread_cond_signal (a->match);
      VERBOSE_PRINT ("paper available\n");
      uthread_cond_signal (a->paper);
      break;
    case 1:
      signal_count[PAPER]++;
      VERBOSE_PRINT ("match available\n");
      uthread_cond_signal (a->match);
      VERBOSE_PRINT ("tobacco available\n");
      uthread_cond_signal (a->tobacco);
      break;
    case 2:
      signal_count[MATCH]++;
      VERBOSE_PRINT ("paper available\n");
      uthread_cond_signal (a->paper);
      VERBOSE_PRINT ("tobacco available\n");
      uthread_cond_signal (a->tobacco);
      break;
    case 3:
      signal_count[TOBACCO]++;
      VERBOSE_PRINT ("paper available\n");
      uthread_cond_signal (a->paper);
      VERBOSE_PRINT ("match available\n");
      uthread_cond_signal (a->match);
      break;
    case 4:
      signal_count[PAPER]++;
      VERBOSE_PRINT ("tobacco available\n");
      uthread_cond_signal (a->tobacco);
      VERBOSE_PRINT ("match available\n");
      uthread_cond_signal (a->match);
      break;
    case 5:
      signal_count[MATCH]++;
      VERBOSE_PRINT ("tobacco available\n");
      uthread_cond_signal (a->tobacco);
      VERBOSE_PRINT ("paper available\n");
      uthread_cond_signal (a->paper);
      break;
    }
    VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
    uthread_cond_wait (a->smoke);
  }
  
  uthread_mutex_unlock (a->mutex);
  return NULL;
}

int main (int argc, char** argv) {
  printf("main is running...\n");
  a = createAgent();
  helper = createHelper(a);
  st = createSmoker(a, helper, TOBACCO);
  sp = createSmoker(a, helper, PAPER);
  sm = createSmoker(a, helper, MATCH);

  uthread_init(8);
  
  // // TODO
  agent_thread = uthread_create(agent, a);
  hm_thread = uthread_create(helperMatch, helper);
  hp_thread = uthread_create(helperPaper, helper);
  ht_thread = uthread_create(helpertobacco, helper);
  st_thread = uthread_create(tobaccoSmoke, st);
  sp_thread = uthread_create(paperSmoke, sp);
  sm_thread = uthread_create(matchSmoke, sm);

  
  uthread_join(agent_thread, NULL);
  // uthread_join(st_thread, NULL);
  // uthread_join(sp_thread, NULL);
  // uthread_join(sm_thread, NULL);
  // uthread_join(hm_thread, NULL);
  // uthread_join(hp_thread, NULL);
  // uthread_join(ht_thread, NULL);


  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);

  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);

  return 0;
}
