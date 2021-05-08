#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_THREADS 50

uthread_mutex_t     mutex;
uthread_cond_t      cond;

void *thread_func(void *arg) {
  for(int i=0; i<100; i++)
    uthread_yield();
  return NULL;
}

void mysrandomdev() {
  unsigned long seed;
  int f = open ("/dev/urandom", O_RDONLY);
  read    (f, &seed, sizeof (seed));
  close   (f);
  srandom (seed);
}

int main (int argc, char** argv) {
  mysrandomdev();

  uthread_init (16);
  uthread_t threads[NUM_THREADS] = {0};
  for(int i=0; i<1000; i++) {
    int j = random() % NUM_THREADS;
    if(threads[j]) {
      uthread_join(threads[j], NULL);
      threads[j] = NULL;
    } else {
      threads[j] = uthread_create(thread_func, NULL);
    }
  }
  for(int j=0; j<NUM_THREADS; j++) {
    if(threads[j]) {
      uthread_join(threads[j], NULL);
      threads[j] = NULL;
    }
  }
}
