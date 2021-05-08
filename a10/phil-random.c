#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define MAX_THINKING_TIME 25000

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__)
#else
#define VERBOSE_PRINT(S, ...) ((void) 0) // do nothing
#endif

typedef struct fork {
  uthread_mutex_t lock;
  uthread_cond_t forfree;
  long free;
} fork_t;

int num_phils, num_meals;    
fork_t *forks;

void deep_thoughts() {
  usleep(random() % MAX_THINKING_TIME);
}

void initfork(int i) {
  forks[i].lock    = uthread_mutex_create();
  forks[i].forfree = uthread_cond_create(forks[i].lock);
  forks[i].free    = 1;
}

long getfork(long i) {
  return forks[i].free;
}

void putfork(long i) {
  forks[i].free = 1;
  uthread_mutex_unlock(forks[i].lock);
}

int leftfork(long i) {
  return i;
}

int rightfork(long i) {
  return (i + 1) % num_phils;
}

void *phil_thread(void *arg) {
  uintptr_t id = (uintptr_t) arg;
  int meals = 0;
  int hasForks = 0;
  
  while (meals < num_meals) {
    // think each time before starting to eat
    deep_thoughts();
    hasForks = 0;
    while (!hasForks) {
        if ((rand() % 2) == 0) {
            // lock left fork; will block until mutex is available
            uthread_mutex_lock(forks[leftfork(id)].lock);
            // if left fork is free, pick it up
            if (getfork(leftfork(id)) == 1) {
                forks[leftfork(id)].free = 0;
                VERBOSE_PRINT("random number = 0; left fork picked up by phil %d\n", id);
            }
            // think after getting one fork
            deep_thoughts();
            // if right fork is free, pick it up
            if (getfork(rightfork(id)) == 1) {
                uthread_mutex_lock(forks[rightfork(id)].lock);
                forks[rightfork(id)].free = 0;
                VERBOSE_PRINT("second (right) fork picked up by phil %d\n", id);
                hasForks = 1;
            } 
            // if right fork is not free, put left fork back and unlock
            else {
                putfork(leftfork(id));
                VERBOSE_PRINT("right fork not available; first (left) fork put down by phil %d\n", id);
            }
        } 
        else {
            // lock right fork; will block until mutex is available
            uthread_mutex_lock(forks[rightfork(id)].lock);
            // if right fork is free, pick it up
            if (getfork(rightfork(id)) == 1) {
                forks[rightfork(id)].free = 0;
                VERBOSE_PRINT("random number = 1; right fork picked up by phil %d\n", id);
            }
            // think after getting one fork
            deep_thoughts();
            // if left fork is free, pick it up
            if (getfork(leftfork(id)) == 1) {
                uthread_mutex_lock(forks[leftfork(id)].lock);
                forks[leftfork(id)].free = 0;
                VERBOSE_PRINT("second (left) fork picked up by phil %d\n", id);
                hasForks = 1;
            } 
            // if left fork is not free, put right fork back and unlock
            else {
                putfork(rightfork(id));
                VERBOSE_PRINT("left fork not available; first (right) fork put down by phil %d\n", id);
            }
        }
    }
    // think after getting both forks
    deep_thoughts();
    // eat (inc meals)
    meals++;
    VERBOSE_PRINT("meal eaten by phil %d. Has eaten %d meals\n", id, meals);
    // think after eating (? unclear if this is required)
    deep_thoughts();
    // put forks down
    putfork(rightfork(id));
    VERBOSE_PRINT("right fork put down by phil %d\n", id);
    putfork(leftfork(id));
    VERBOSE_PRINT("left fork put down by phil %d\n", id);
    // think after putting the forks down before trying to eat again
    deep_thoughts();
    VERBOSE_PRINT("phil %d finished a cycle\n", id);
  }
  VERBOSE_PRINT("phil %d has finished all %d meals and all deep thoughts\n", id, meals);
  return 0;
}

int main(int argc, char **argv) {

  uthread_t *p;
  uintptr_t i;
  
  if (argc != 3) {
    fprintf(stderr, "Usage: %s num_philosophers num_meals\n", argv[0]);
    return 1;
  }

  num_phils = strtol(argv[1], 0, 0);
  num_meals = strtol(argv[2], 0, 0);
  
  forks = malloc(num_phils * sizeof(fork_t));
  p = malloc(num_phils * sizeof(pthread_t));

  uthread_init(num_phils);

  srandom(time(0));
  for (i = 0; i < num_phils; ++i) {
    initfork(i);
  }

  /* TODO: Create num_phils threads, all calling phil_thread with a
   * different ID, and join all threads.
   */
  for (i=0; i<num_phils; i++)
    p[i] = uthread_create(phil_thread, (void*)i);
  for (i=0; i<num_phils; i++)
    uthread_join(p[i], NULL);
  
  // destroy all fork locks and conditional vars
  for (i = 0; i < num_phils; ++i) {
    uthread_cond_destroy(forks[i].forfree);
    uthread_mutex_destroy(forks[i].lock);
  }

  return 0;
}
