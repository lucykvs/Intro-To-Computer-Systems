#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"

int numProcessors;
int numThreads;
int numIterations;
int numYields;
int useMonitors;
uthread_monitor_t* m;

void* testThread () {
  for (int i=0; i<numIterations; i++) {
    if (useMonitors) uthread_monitor_enter (m);
      for (int j=0; j<numYields; j++)
        uthread_yield();
    if (useMonitors) uthread_monitor_exit (m);
  }
  return NULL;
}

int main (int argc, char** argv) {
  assert (argc==6);
  numProcessors = strtol (argv[1], 0, 10);
  numThreads    = strtol (argv[2], 0, 10);
  numIterations = strtol (argv[3], 0, 10);
  numYields     = strtol (argv[4], 0, 10);
  useMonitors   = strtol (argv[5], 0, 10);
  printf ("p=%d t=%d i=%d y=%d m=%d\n", numProcessors, numThreads, numIterations, numYields, useMonitors);
  uthread_init (numProcessors);
  m = uthread_monitor_create();
  uthread_t** t = malloc (numThreads * sizeof (uthread_t*));
  for (int i=0; i<numThreads; i++)
    t [i] = uthread_create (testThread, NULL);
  for (int i=0; i<numThreads; i++)
    uthread_join (t [i]);
}