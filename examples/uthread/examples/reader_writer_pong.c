//
// Written by Mike Feeley, University of BC, 2010
// Do not redistribute or otherwise make available any portion of this code to anyone without written permission of the author.
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "spinlock.h"
#include "uthread.h"

#ifndef NUM_ITERATIONS
#define NUM_ITERATIONS 100
#endif
#ifndef NUM_READERS
#define NUM_READERS    16
#endif
#ifndef NUM_WRITERS
#define NUM_WRITERS    4
#endif
#ifndef NUM_PROCESSORS
#define NUM_PROCESSORS 2
#endif
#ifndef MONITOR_HOLD_WAIT
#define MONITOR_HOLD_WAIT 100000
#endif

struct control_t {
  spinlock_t lock;
  int reader_count;
  int writer_count;
  int reader_histogram[NUM_READERS+NUM_WRITERS];
} control;

uthread_monitor_t* monitor;

void init () {
  int i;
  
  control.reader_count = 0;
  control.writer_count = 0;
  for (i=0; i<NUM_READERS+NUM_WRITERS; i++) 
    control.reader_histogram[i] = 0;
  monitor = uthread_monitor_create ();
}

void count_reader () {
  int i;
  
  spinlock_lock (&control.lock);
  assert (control.writer_count==0);
  control.reader_count ++;
  control.reader_histogram [control.reader_count] ++;
  spinlock_unlock (&control.lock);
  for (i=0; i<MONITOR_HOLD_WAIT; i++) {}
  spinlock_lock (&control.lock);
  control.reader_count --;
  spinlock_unlock (&control.lock);
}

void count_writer () {
  int i;
  
  spinlock_lock (&control.lock);
  assert (control.writer_count==0 && control.reader_count==0);
  control.writer_count ++;
  spinlock_unlock (&control.lock);
  for (i=0; i<MONITOR_HOLD_WAIT; i++) {}
  spinlock_lock (&control.lock);
  control.writer_count --;
  spinlock_unlock (&control.lock);
}

void* reader (void* arg) {
  int i;
  
  for (i=0; i<NUM_ITERATIONS; i++) {
    uthread_monitor_enter_read_only (monitor);
    count_reader ();
    uthread_monitor_exit (monitor);
  }
  return NULL;
}

void* writer (void* arg) {
  int i;
  
  for (i=0; i<NUM_ITERATIONS; i++) {
    uthread_monitor_enter (monitor);
    count_writer ();
    uthread_monitor_exit  (monitor);
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t* t[NUM_READERS+NUM_WRITERS];
  int i;
  
  uthread_init (NUM_PROCESSORS);
  init         ();
  for (i=0; i<NUM_READERS; i++) 
    t[i] = uthread_create (reader,0);
  for (i=0; i<NUM_WRITERS; i++) 
    t[NUM_READERS+i] = uthread_create (writer,0);
  for (i=0; i<NUM_READERS; i++) 
    uthread_join (t[i]);
  for (i=0; i<NUM_WRITERS; i++) 
    uthread_join (t[NUM_READERS+i]);
  
  printf ("reader_count histogram:\n");
  for (i=0; i<NUM_READERS+NUM_WRITERS; i++)
    if (control.reader_histogram [i])
      printf ("%d:\t%d\n",i,control.reader_histogram[i]);
}
