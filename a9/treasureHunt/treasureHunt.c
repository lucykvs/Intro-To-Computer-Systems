#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <assert.h>
#include "uthread.h"
#include "queue.h"
#include "disk.h"

queue_t pending_read_queue;
volatile int is_read_pending;
volatile int is_program_running = 1;

void interrupt_service_routine() {
  is_read_pending = 0;
}

void handleOtherReads(void *resultv, void *countv) {
  // TODO
}

void handleFirstRead(void *resultv, void *countv) {
  // TODO
}

int main(int argc, char **argv) {
  // Command Line Arguments
  static char* usage = "usage: treasureHunt starting_block_number";
  int starting_block_number;
  char *endptr;
  if (argc == 2)
    starting_block_number = strtol (argv [1], &endptr, 10);
  if (argc != 2 || *endptr != 0) {
    printf ("argument error - %s \n", usage);
    return EXIT_FAILURE;
  }

  // Initialize
  uthread_init (1);
  disk_start (interrupt_service_routine);
  pending_read_queue = queue_create();


  // Start the Hunt
  // TODO
  int counter;
  int result;
  is_read_pending = 1;
  disk_schedule_read (&result, starting_block_number);
  while(is_read_pending);

  counter = result;

  // Sum Blocks
  for (int readno = 0; readno < counter; readno++) {
    is_read_pending = 1;                     // set flag saying that read will be pending
    disk_schedule_read (&result, result);   // request disk to read specified blockno
    
    while (is_read_pending);                 // loop until is_read_pending == 0
  }

  printf("%d\n", result);
  is_program_running = 0;

  while (is_program_running); // infinite loop so that main doesn't return before hunt completes
}
