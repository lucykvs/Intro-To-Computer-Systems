#include <stdio.h>
#include <stdlib.h>

#include "uthread.h"

uthread_t t1, t2, t3;

void *thread_func(void *arg) {
  char *msg = arg;
  printf("hi from %s\n", msg);
}

int main() {
  uthread_init(1);
  t1 = uthread_create(thread_func, "t1");
  t2 = uthread_create(thread_func, "t2");
  t3 = uthread_create(thread_func, "t3");
  uthread_unblock(t1);
  uthread_unblock(t2);
  uthread_unblock(t3);
  uthread_unblock(t1);
  uthread_join(t1, NULL);
  uthread_join(t2, NULL);
  uthread_join(t3, NULL);
}
