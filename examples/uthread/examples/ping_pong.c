#include <stdlib.h>
#include <stdio.h>
#include "uthread.h"

#include <unistd.h>

#define NUM_ITERATIONS 100

void* ping (void* x) {
  int i,j;
  for (i=0; i<NUM_ITERATIONS; i++) {
    printf ("I"); fflush(stdout);
    for (j=0; j<100000; j++) {}
    uthread_yield ();
  }
  return NULL;
}

void* pong (void* x) {
  int i,j;
  for (i=0; i<NUM_ITERATIONS; i++) {
    printf ("O"); fflush(stdout);
    for (j=0; j<100000; j++) {}
    uthread_yield ();
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t0, t1;
  int i;
  
  uthread_init (2);
  t0 = uthread_create (ping, 0);
  t1 = uthread_create (pong, 0);
  for (int j=0; j<1000000; j++) {}
  uthread_join (t0, 0);
  uthread_join (t1, 0);

  printf ("\n");
}
