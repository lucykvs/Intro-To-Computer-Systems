#include <stdlib.h>
#include <stdio.h>
#include "uthread.h"

int n;
int array [2];

void check_n() {
  if (n < 0 || n > 2) {
    fprintf (stderr, "PANIC: n out of bounds: %d\n", n);
    exit (-1);
  }
}

void push (int i) {
  if (n < 2) {
    array [n] = i;
    n++;
  }
  check_n();
}

int pop() {
  if (n > 0) {
    n--;
    check_n();
    return array [n];
  } else
    return -1;
}

void* driver (void * v) {
  for (int i=0; i<1000000; i++) {
    if (random() & 1)
      push (i);
    else
      pop();
  }
  return 0;
}

int main (int argc, char** argv) {
  uthread_init(2);
  uthread_t t0 = uthread_create (driver, 0);
  uthread_t t1 = uthread_create (driver, 0);
  uthread_join (t0, 0);
  uthread_join (t1, 0);
}
