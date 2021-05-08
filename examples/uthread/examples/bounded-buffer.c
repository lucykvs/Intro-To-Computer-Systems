//
// Written by Mike Feeley, University of BC, 2010-2014
// Do not redistribute any portion of this code without permission.
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#ifndef NUM_PROCESSORS
#define NUM_PROCESSORS 4
#endif
#ifndef NUM_THREAD_PAIRS
#define NUM_THREAD_PAIRS 2
#endif
#ifndef QUEUE_SIZE
#define QUEUE_SIZE 16
#endif
#ifndef NUM_ITERATIONS
#define NUM_ITERATIONS (QUEUE_SIZE*4)
#endif
#ifndef PRINT_EACH_THREAD_DONE
#define PRINT_EACH_THREAD_DONE 1
#endif
#ifndef PRINT_EACH_ITERATION
#define PRINT_EACH_ITERATION 1
#endif


/**
 * Bounded Buffer
 */

int queue [QUEUE_SIZE], queue_head=0, queue_tail=0;
uthread_mutex_t queue_mutex;
uthread_cond_t  queue_not_empty;
uthread_cond_t  queue_not_full;


/**
 * queue_init
 */

void queue_init () {
  queue_mutex     = uthread_mutex_create ();
  queue_not_empty = uthread_cond_create  (queue_mutex);
  queue_not_full  = uthread_cond_create  (queue_mutex);
}


/**
 * queue_enqueue
 */

void queue_enqueue (int val) {
  int i;
  uthread_mutex_lock (queue_mutex);
    while ( (queue_tail+1) % QUEUE_SIZE == queue_head) {
      uthread_cond_wait (queue_not_full);
    }
    queue[queue_tail] = val;
    queue_tail        = (queue_tail + 1) % QUEUE_SIZE;
    uthread_cond_signal (queue_not_empty);
  uthread_mutex_unlock (queue_mutex);
}


/**
 * queue_dequeue
 */

int queue_dequeue () {
  int i;
  int val;
  
  uthread_mutex_lock (queue_mutex);
    while (queue_head == queue_tail) {
      uthread_cond_wait (queue_not_empty);
    }
    val        = queue[queue_head];
    queue_head = (queue_head + 1) % QUEUE_SIZE;
    uthread_cond_signal (queue_not_full);
  uthread_mutex_unlock (queue_mutex);
  
  return val;
}


/**
 * producer
 */

void* producer (void* id) {
  int i;
  for (i=0; i<NUM_ITERATIONS; i++)
    queue_enqueue (((uintptr_t) id)*1000 + i);
#if PRINT_EACH_THREAD_DONE
  printf ("p %ld done\n", (uintptr_t) id); fflush (stdout);
#endif
  return NULL;
}


/**
 * consumer
 */

void* consumer (void* id) {
  int i;
  for (i=0; i<NUM_ITERATIONS; i++)
#if PRINT_EACH_ITERATION
    printf ("%d consumed by %ld(%d)\n", queue_dequeue(), (uintptr_t) id, i);
#else
    queue_dequeue();
#endif
#if PRINT_EACH_THREAD_DONE
  printf ("c %ld done\n",(uintptr_t) id); fflush (stdout);
#endif
  return NULL;
}


/**
 * main
 */

int main (int argc, char** argv) {
  uintptr_t i;
  uthread_t pt[NUM_THREAD_PAIRS], ct[NUM_THREAD_PAIRS];
  uthread_init (NUM_PROCESSORS);
  queue_init ();
  for (i=0; i<NUM_THREAD_PAIRS; i++) {
    pt[i] = uthread_create (producer,(void*) i);
    ct[i] = uthread_create (consumer,(void*) i);
  }
  for (i=0; i<NUM_THREAD_PAIRS; i++) {
    uthread_join (pt[i], 0);
    uthread_join (ct[i], 0);
  }
}