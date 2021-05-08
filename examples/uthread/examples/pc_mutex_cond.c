//
// This is the course-provided solution to A8.  Do not distribute.
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define MAX_ITEMS      10
#define NUM_ITERATIONS 100
#define NUM_THREADS    4

int producer_wait_count;
int consumer_wait_count;
int histogram [MAX_ITEMS + 1];

struct Pool {
  uthread_mutex_t mutex;
  uthread_cond_t  notEmpty;
  uthread_cond_t  notFull;
  int             items;
};

struct Pool* createPool() {
  struct Pool* pool = malloc (sizeof (struct Pool));
  pool->mutex    = uthread_mutex_create();
  pool->notEmpty = uthread_cond_create (pool->mutex);
  pool->notFull  = uthread_cond_create (pool->mutex);
  pool->items    = 0;
  return pool;
}

void* producer (void* pv) {
  struct Pool* p = pv;
  
  for (int i=0; i<NUM_ITERATIONS; i++) {
    uthread_mutex_lock (p->mutex);
      while (p->items == MAX_ITEMS) {
        producer_wait_count++;
        uthread_cond_wait (p->notFull);
      }
      p->items++;
      uthread_cond_signal (p->notEmpty);
      assert (p->items >= 0 && p->items <= MAX_ITEMS);
      histogram [p->items] ++;
    uthread_mutex_unlock (p->mutex);
  }
  return NULL;
}

void* consumer (void* pv) {
  struct Pool* p = pv;
  
  for (int i=0; i<NUM_ITERATIONS; i++) {
    uthread_mutex_lock (p->mutex);
      while (p->items == 0) {
        consumer_wait_count++;
        uthread_cond_wait (p->notEmpty);
      }
      p->items--;
      uthread_cond_signal (p->notFull);
      assert (p->items >= 0 && p->items <= MAX_ITEMS);
      histogram [p->items] ++;
    uthread_mutex_unlock (p->mutex);
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_init (4);
  
  struct Pool* p = createPool();
  uthread_t t[4];
  
  for (int i = 0; i < sizeof (t) / sizeof (uthread_t); i++)
    t [i] = uthread_create ((i & 1)? producer : consumer, p);
  for (int i = 0; i < sizeof (t) / sizeof (uthread_t); i++)
    uthread_join (t [i], 0);
  
  printf ("producer_wait_count=%d, consumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}