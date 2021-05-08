#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/errno.h>
#include <assert.h>
#include "queue.h"
#include "disk.h"
#include "uthread.h"

queue_t      pending_read_queue;
unsigned int sum = 0;

void interrupt_service_routine () {
  void* val;
  void (*callback) (void*,void*);
  queue_dequeue(pending_read_queue, &val, NULL, &callback);
  uthread_unblock((uthread_t)val);
}

void* read_block (void* blocknov) {
  // TODO schedule read and the update (correctly)
  int blockno = *((int*) blocknov);
  int resultBuf;
  disk_schedule_read(&resultBuf, blockno);
  void (*callback) (void*,void*);
  queue_enqueue(pending_read_queue, (void*)uthread_self(), NULL, callback);
  uthread_block();
  sum += resultBuf;
  return NULL;
}

int main (int argc, char** argv) {

  // Command Line Arguments
  static char* usage = "usage: tRead num_blocks";
  int num_blocks;
  char *endptr;
  if (argc == 2)
    num_blocks = strtol (argv [1], &endptr, 10);
  if (argc != 2 || *endptr != 0) {
    printf ("argument error - %s \n", usage);
    return EXIT_FAILURE;
  }

  // Initialize
  uthread_init (1);
  disk_start (interrupt_service_routine);
  pending_read_queue = queue_create();

  // Sum Blocks
  // TODO

  uthread_t threads[num_blocks];
  // create new thread for each call to read
  for (int blockno = 0; blockno < num_blocks; blockno++) {
    threads[blockno] = uthread_create(&read_block, (void*)&blockno);
  }

  // join threads after all have been created
  for (int blockno = 0; blockno < num_blocks; blockno++) {
    uthread_join(threads[blockno], 0);
  }

  printf("%d\n", sum);
}

