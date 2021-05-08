#include <stdio.h>
#include <assert.h>
#include "uthread.h"

struct sum_args {
  int *a;
  int s;
};

void* sum (void* args_v) {
  struct sum_args *args = (struct sum_args*) args_v; 
  int i, sum = 0;
  for (i=0; i<args->s; i++)
    sum += args->a[i];
  return (void*) (long int) sum;
}


int par_ave (int *a, int s) {
  struct sum_args a1, a2;
  long int        r1, r2;
  uthread_t       *t1, *t2;
  
  a1.a = a;
  a1.s = s/2;
  t1   = uthread_create (sum, (void *) &a1);
  a2.a = a + a1.s;
  a2.s = s-a1.s;
  t2   = uthread_create (sum, (void *) &a2);
  r1   = (long int) uthread_join (t1);
  r2   = (long int) uthread_join (t2);
  return (r1+r2)/s;
}

int array[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90 };

int main (int argc, char** argv) {
  uthread_init (2);
  printf ("ave=%d\n", par_ave (array, sizeof(array) / sizeof(array[0])));
}
