//
// Written by Mike Feeley, University of BC, 2010-2014
// Do not redistribute any portion of this code without permission.
//

#include <stdlib.h>
#include <stdio.h>
#include "uthread.h"
#include "uthread_sem.h"

uthread_sem_t go_ping, go_pong;

void* ping () {
  int i;
  for (i=0; i<10; i++) {
    uthread_sem_signal (go_pong);
    uthread_sem_wait   (go_ping);
    printf ("ping %d\n",i);
  }
  return NULL;
}

void* pong () {
  int i;
  for (i=0; i<10; i++) {
    uthread_sem_signal (go_ping);
    uthread_sem_wait   (go_pong);
    printf ("pong %d\n",i);
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t ping_thread, pong_thread;
  int i;
  uthread_init (2);
  go_ping = uthread_sem_create (0);
  go_pong = uthread_sem_create (0);
  ping_thread = uthread_create (ping, 0);
  pong_thread = uthread_create (pong, 0);
  uthread_join (ping_thread, 0);
  uthread_join (pong_thread, 0);
}
