#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_THREADS 20

uthread_mutex_t     mutex;
uthread_cond_t      cond;

void *thread_func(void *arg) {
  for(int i=0; i<2000; i++)
    uthread_yield();
  return NULL;
}

int main (int argc, char** argv) {
  uthread_init (16);
  uthread_t threads[NUM_THREADS];
  for(int i=0; i<NUM_THREADS; i++) {
    threads[i] = uthread_create(thread_func, NULL);
  }
  for(int i=0; i<NUM_THREADS; i++) {
    uthread_join(threads[i], NULL);
  }
}
