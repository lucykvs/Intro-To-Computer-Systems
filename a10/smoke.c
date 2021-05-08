#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_ITERATIONS 1000

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__)
#else
#define VERBOSE_PRINT(S, ...) ((void) 0) // do nothing
#endif

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

// # of threads waiting for a signal. Used to ensure that the agent
// only signals once all other threads are ready.
int num_active_threads = 0;

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked

/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can modify it if you like, but be sure that all it does
 * is choose 2 random resources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agent (void* av) {
  struct Agent* a = av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};

  srandom(time(NULL));
  
  uthread_mutex_lock (a->mutex);
  // Wait until all other threads are waiting for a signal
  while (num_active_threads < 3)
    uthread_cond_wait (a->smoke);

  for (int i = 0; i < NUM_ITERATIONS; i++) {
    int r = random() % 6;
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

// // one "resources" struct for the whole problem
// // keep track of whether match, paper, tobacco is available; ints act as booleans
// // if one smoker is unable to smoke with the resources available, will increment its resource
// typedef struct Resources {
//   struct Agent* agent;
//   int match, paper, tobacco;
// } resources_t;

// // initialize resources struct such that 
// resources_t* initResources(struct Agent* agent) {
//   resources_t* resources = malloc(sizeof (resources_t));
//   resources->agent = agent;
//   resources->match = 0;
//   resources->paper = 0;
//   resources->tobacco = 0;
//   return resources;
// }

typedef struct Helper {
  struct Agent* agent;
  uthread_cond_t tobaccoCanGo;
  uthread_cond_t paperCanGo;
  uthread_cond_t matchCanGo;
  int match, paper, tobacco;
} helper_t;

typedef struct Smoker {
  helper_t* helper;
  int type;
} smoker_t;

smoker_t* createSmoker(int type, helper_t* helper) {
  struct Smoker* smoker = malloc(sizeof (struct Smoker));
  smoker->type = type;
  smoker->helper = helper;
  return smoker;
}

helper_t* createHelper(struct Agent* agent) {
  helper_t* helper = malloc(sizeof(helper_t));
  helper->agent = agent;
  helper->tobaccoCanGo = uthread_cond_create(agent->mutex);
  helper->paperCanGo = uthread_cond_create(agent->mutex);
  helper->matchCanGo = uthread_cond_create(agent->mutex);
  helper->match = 0;
  helper->paper = 0;
  helper->tobacco = 0;
  return helper;
}

void* helper_smoker_match(void* h) {
  helper_t* helper = h;
  struct Agent* a = helper->agent;
  uthread_mutex_lock(a->mutex);
  VERBOSE_PRINT ("helper_smoker_match is running\n");
  // infinite loop for helper to repeatedly check for available resource pair
  // while(1) {
    VERBOSE_PRINT ("helper_smoker_match is waiting\n");
    uthread_cond_wait(a->paper);
    helper->paper++;
    if (helper->paper && helper->tobacco) {
      VERBOSE_PRINT ("paper and tobacco available; signalling matchCanGo\n");
      uthread_cond_signal(helper->matchCanGo);
      helper->paper--;
      helper->tobacco--;
    }
  // }
  uthread_mutex_unlock(a->mutex);
}

void* helper_smoker_paper(void* h) {
  helper_t* helper = h;
  struct Agent* a = helper->agent;
  uthread_mutex_lock(a->mutex);
  VERBOSE_PRINT ("helper_smoker_paper is running\n");
  // infinite loop for helper to repeatedly check for available resource pair
  // while(1) {
    VERBOSE_PRINT ("helper_smoker_paper is waiting\n");
    uthread_cond_wait(a->tobacco);
    helper->tobacco++;
    if (helper->tobacco && helper->match) {
      VERBOSE_PRINT ("tobacco and match available; signalling paperCanGo\n");
      uthread_cond_signal(helper->paperCanGo);
      helper->tobacco--;
      helper->paper--;
    }
  // }
  uthread_mutex_unlock(a->mutex);
}

void* helper_smoker_tobacco(void* h) {
  helper_t* helper = h;
  struct Agent* a = helper->agent;
  uthread_mutex_lock(a->mutex);
  VERBOSE_PRINT ("helper_smoker_tobacco is running\n");
  // infinite loop for helper to repeatedly check for available resource pair
  // while(1) {
    VERBOSE_PRINT ("helper_smoker_tobacco is waiting\n");
    uthread_cond_wait(a->match);
    helper->match++;
    if (helper->match && helper->paper) {
      VERBOSE_PRINT ("match and paper available; signalling tobaccoCanGo\n");
      uthread_cond_signal(helper->tobaccoCanGo);
      helper->match--;
      helper->paper--;
    }
  // }
  uthread_mutex_unlock(a->mutex);
}

void* tobacco_smoker(void* s) {
  smoker_t* smoker = s;
  helper_t* helper = smoker->helper;
  struct Agent* a = smoker->helper->agent;
  uthread_mutex_lock(a->mutex);
  VERBOSE_PRINT ("tobacco_smoker is running\n");
  num_active_threads++;
  // infinite loop for smoker to wait on tobaccoCanGo var
  while(1) {
    // wait until helper signals resources for tobacco smoker are available
    VERBOSE_PRINT ("tobacco_smoker is waiting\n");
    uthread_cond_wait(helper->tobaccoCanGo);
    // inc number of times this smoker has smoked
    smoke_count[TOBACCO]++;
    VERBOSE_PRINT ("tobacco_smoker has smoked %d times\n", smoke_count[TOBACCO]);
    uthread_cond_signal(a->smoke);
  }
  uthread_mutex_unlock(a->mutex);
}

void* paper_smoker(void* s) {
  smoker_t* smoker = s;
  helper_t* helper = smoker->helper;
  struct Agent* a = smoker->helper->agent;
  uthread_mutex_lock(a->mutex);
  VERBOSE_PRINT ("paper_smoker is running\n");
  num_active_threads++;
  // infinite loop for smoker to wait on paperCanGo cond var
  while(1) {
    // wait until agent signals resources for paper smoker are available
    VERBOSE_PRINT ("paper_smoker is waiting\n");
    uthread_cond_wait(helper->paperCanGo);
    // inc number of times this smoker has smoked
    smoke_count[PAPER]++;
    VERBOSE_PRINT ("paper_smoker has smoked %d times\n", smoke_count[PAPER]);
    uthread_cond_signal(a->smoke);
  }
  uthread_mutex_unlock(a->mutex);
}

void* matches_smoker(void* s) {
  smoker_t* smoker = s;
  helper_t* helper = smoker->helper;
  struct Agent* a = smoker->helper->agent;
  uthread_mutex_lock(a->mutex);
  VERBOSE_PRINT ("matches_smoker is running\n");
  num_active_threads++;
  // infinite loop for smoker to repeatedly wait on matchCanGo cond var
  while(1) {
    // wait until agent signals resources for match smoker are available
    VERBOSE_PRINT ("match_smoker is waiting\n");
    uthread_cond_wait(helper->matchCanGo);
    // inc number of times this smoker has smoked
    smoke_count[MATCH]++;
    VERBOSE_PRINT ("match_smoker has smoked %d times\n", smoke_count[MATCH]);
    uthread_cond_signal(a->smoke);
  }
  uthread_mutex_unlock(a->mutex);
}

int main (int argc, char** argv) {
  struct Agent* a = createAgent();
  helper_t* helper = createHelper(a);
  smoker_t* t_smoker = createSmoker(TOBACCO, helper);
  smoker_t* p_smoker = createSmoker(PAPER, helper);
  smoker_t* m_smoker = createSmoker(MATCH, helper);

  uthread_t agent_thread, smoker_tobacco, smoker_paper, smoker_matches, hm_thread, hp_thread, ht_thread;

  uthread_init(7);

  smoker_tobacco = uthread_create(tobacco_smoker, t_smoker);
  smoker_paper = uthread_create(paper_smoker, p_smoker);
  smoker_matches = uthread_create(matches_smoker, m_smoker);
  hm_thread = uthread_create(helper_smoker_match, helper);
  hp_thread = uthread_create(helper_smoker_paper, helper);
  ht_thread = uthread_create(helper_smoker_tobacco, helper);

  agent_thread = uthread_create(agent, a);
  uthread_join(agent_thread, NULL);

  uthread_detach(smoker_tobacco);
  uthread_detach(smoker_paper);
  uthread_detach(smoker_matches);
  uthread_detach(hm_thread);
  uthread_detach(hp_thread);
  uthread_detach(ht_thread);

  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);

  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);

  return 0;
}
