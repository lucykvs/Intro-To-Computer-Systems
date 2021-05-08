#include <stdlib.h>
#include <stdio.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

uthread_t t0, t1, t2;
uthread_mutex_t mx;
uthread_cond_t cond1;
uthread_cond_t cond2;
int canGo1;
int canGo2;

void randomStall() {
  int i, r = random() >> 16;
  while (i++<r);
}

void* p0(void* v) {
  uthread_mutex_lock(mx);
  randomStall();
  printf("zero\n");
  canGo1 = 1;
  uthread_cond_signal(cond1);
  uthread_mutex_unlock(mx);
  return NULL;
}

void* p1(void* v) {
  uthread_mutex_lock(mx);
  
  while(!canGo1)
    uthread_cond_wait(cond1);

  randomStall();
  printf("one\n");
  canGo2 = 1;
  uthread_cond_signal(cond2);
  uthread_mutex_unlock(mx);
  return NULL;
}

void* p2(void* v) {
  uthread_mutex_lock(mx);

  while(!canGo2)
    uthread_cond_wait(cond2);

  randomStall();
  printf("two\n");
  uthread_mutex_unlock(mx);
  return NULL;
}

int main(int arg, char** arv) {
  uthread_init(4);
  mx = uthread_mutex_create();
  cond1 = uthread_cond_create(mx);
  cond2 = uthread_cond_create(mx);
  canGo1 = 0;
  canGo2 = 0;
  t0 = uthread_create(p0, NULL);
  t1 = uthread_create(p1, NULL);
  t2 = uthread_create(p2, NULL);
  randomStall();
  uthread_join (t0, NULL);
  uthread_join (t1, NULL);
  uthread_join (t2, NULL);
  printf("three\n");
  printf("------\n");
  uthread_cond_destroy(cond1);
  uthread_cond_destroy(cond2);
  uthread_mutex_destroy(mx);
}
