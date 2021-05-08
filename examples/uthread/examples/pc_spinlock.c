//
// This is the course-provided solution to A8.  Do not distribute.
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "spinlock.h"
#include "uthread.h"

const int MAX_ITEMS = 10;
int       items     = 0;
spinlock_t items_lock;

const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;
spinlock_t pwc_lock;
int consumer_wait_count;
spinlock_t cwc_lock;

int histogram[11];

void produce() {
  for(;;) {
    while(items == MAX_ITEMS) {
      spinlock_lock(&pwc_lock);
      producer_wait_count++;
      spinlock_unlock(&pwc_lock);
    }
    spinlock_lock(&items_lock);
    if(items < MAX_ITEMS)
      break;
    spinlock_unlock(&items_lock);
  }
  items++;
  histogram[items]++;
  assert (items <= MAX_ITEMS);
  spinlock_unlock(&items_lock);
}

void consume() {
  for(;;) {
    while(!items) {
      spinlock_lock(&cwc_lock);
      consumer_wait_count++;
      spinlock_unlock(&cwc_lock);
    }
    spinlock_lock(&items_lock);
    if(items)
      break;
    spinlock_unlock(&items_lock);
  }
  items--;
  histogram[items]++;
  assert (items >= 0);
  spinlock_unlock(&items_lock);
}

void *producer(void *a) {
  for (int i=0; i < NUM_ITERATIONS; i++)
    produce();
  return 0;
}

void *consumer(void *a) {
  for (int i=0; i< NUM_ITERATIONS; i++)
    consume();
  return 0;
}

int main (int argc, char** argv) {
  uthread_t prods[NUM_PRODUCERS];
  uthread_t cons[NUM_CONSUMERS];
  int i;
  uthread_init(16);
  spinlock_create(&items_lock);
  spinlock_create(&pwc_lock);
  spinlock_create(&cwc_lock);
  for(i=0;i<NUM_PRODUCERS;i++)
    prods[i] = uthread_create(producer, 0);
  for(i=0;i<NUM_CONSUMERS;i++)
    cons[i] = uthread_create(consumer, 0);
  for(i=0;i<NUM_CONSUMERS;i++)
    uthread_join(cons[i], 0);
  for(i=0;i<NUM_PRODUCERS;i++)
    uthread_join(prods[i], 0);
  printf("Producer wait: %d\nConsumer wait: %d\n",
         producer_wait_count, consumer_wait_count);
  for(i=0;i<MAX_ITEMS+1;i++)
    printf("items %d count %d\n", i, histogram[i]);
}
