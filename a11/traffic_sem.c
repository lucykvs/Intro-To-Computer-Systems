#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include "uthread.h"
#include "uthread_sem.h"

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__)
#else
#define VERBOSE_PRINT(S, ...) ((void) 0) // do nothing
#endif

#define MAX_OCCUPANCY  3
#define NUM_ITERATIONS 100
#define NUM_CARS       20

// These times determine the number of times yield is called when in
// the street, or when waiting before crossing again.
#define CROSSING_TIME             NUM_CARS
#define WAIT_TIME_BETWEEN_CROSSES NUM_CARS

/**
 * You might find these declarations useful.
 */
enum Direction {EAST = 0, WEST = 1};
const static enum Direction oppositeEnd [] = {WEST, EAST};

struct Street {
  uthread_sem_t lock;
  enum Direction direction;         // current direction of cars in street
  uthread_sem_t dir_sems[2];        // to wait and signal cars (EAST, WEST)
  int count;                        // number of cars in street
  int num_each_dir_in_street[2];    // keeps track of cars in street (EAST, WEST)
  int num_each_dir_waiting[2];      // keeps track of cars waiting to enter (EAST, WEST)
  int num_active_each_dir[2];       // number of active threads in each direction (EAST, WEST)
} Street;

void initializeStreet(void) {
  Street.lock = uthread_sem_create(1);
  Street.direction = 0;
  Street.dir_sems[EAST] = uthread_sem_create(0);
  Street.dir_sems[WEST] = uthread_sem_create(0);
  Street.count = 0;
  Street.num_each_dir_in_street[EAST] = 0;
  Street.num_each_dir_in_street[WEST] = 0;
  Street.num_each_dir_waiting[EAST] = 0;
  Street.num_each_dir_waiting[WEST] = 0;
  Street.num_active_each_dir[0] = 0;
  Street.num_active_each_dir[1] = 0;
}

#define WAITING_HISTOGRAM_SIZE (NUM_ITERATIONS * NUM_CARS)
int             entryTicker;                                          // incremented with each entry
int             waitingHistogram [WAITING_HISTOGRAM_SIZE];
int             waitingHistogramOverflow;
uthread_sem_t   waitingHistogramLock;
int             occupancyHistogram [2] [MAX_OCCUPANCY + 1];

void recordWaitingTime (int waitingTime) {
  uthread_sem_wait (waitingHistogramLock);
  if (waitingTime < WAITING_HISTOGRAM_SIZE)
    waitingHistogram [waitingTime] ++;
  else
    waitingHistogramOverflow ++;
  uthread_sem_signal (waitingHistogramLock);
}

void enterStreet (enum Direction g) {
  // lock street as critical section
  uthread_sem_wait(Street.lock);
  int wait_time;
  int t;

  if (g == Street.direction) {
    for (int i = 0; i < MAX_OCCUPANCY - Street.count; i++) {
      uthread_sem_signal(Street.dir_sems[g]);
    }
  }

  // if intersection is empty, set direction to car trying to enter
  if (Street.count == 0 && g != Street.direction) {
    // check that not all cars in direction g are finished
    if (Street.num_active_each_dir[g] != 0) {
      Street.direction = g; 
      for (int i = 0; i < MAX_OCCUPANCY; i++) 
        uthread_sem_signal(Street.dir_sems[Street.direction]);
      VERBOSE_PRINT ("intersection has changed to %s and %d cars have been signalled\n", g==EAST ? "EAST" : "WEST", MAX_OCCUPANCY);
    }
  }

  if (g == Street.direction && Street.num_each_dir_in_street[g] < MAX_OCCUPANCY) {
    // enter street
    VERBOSE_PRINT ("%s-bound car has entered street immediately\n", g==EAST ? "EAST" : "WEST");
    Street.count++; 
    Street.num_each_dir_in_street[g]++;
    // add to waitingHistogram
    recordWaitingTime(0);
    // increment entry time
    entryTicker++;
    // add to occupancyHistogram
    occupancyHistogram[g][Street.count]++;
    uthread_sem_signal(Street.lock);
    return;
  } else {
    // record time before entering and/or waiting
    t = entryTicker;
    Street.num_each_dir_waiting[g]++;
  } 
  
  uthread_sem_signal(Street.lock);   // unlock street
  
  uthread_sem_wait(Street.dir_sems[g]);   // wait

  uthread_sem_wait(Street.lock);        // lock street
  VERBOSE_PRINT ("%s-bound car has entered street after waiting\n", g==EAST ? "EAST" : "WEST");
  // calculate waiting time to enter street
  wait_time = entryTicker - t;
  // add to waitingHistogram
  recordWaitingTime(wait_time);
  Street.num_each_dir_waiting[g]--;
  Street.count++;
  Street.num_each_dir_in_street[g]++;
  entryTicker++;
  occupancyHistogram[g][Street.count]++;
  uthread_sem_signal(Street.lock);   // unlock street
}

void leaveStreet(enum Direction g) {
  // lock street as critical section
  VERBOSE_PRINT ("car going %s is attempting to obtain lock in leaveStreet\n", Street.direction==EAST ? "EAST" : "WEST");
  uthread_sem_wait(Street.lock);
  // decrement num cars in street
  Street.count--;
  Street.num_each_dir_in_street[g]--;
  VERBOSE_PRINT ("there are currently %d %s cars in the street\n",  Street.num_each_dir_in_street[g], g==EAST ? "EAST" : "WEST");
  VERBOSE_PRINT ("there are currently %d EAST cars waiting and %d WEST cars waiting\n", Street.num_each_dir_waiting[0], Street.num_each_dir_waiting[1]);

  // if street is empty and not all cars in opposite direction are finished
  if ((Street.count == 0) && (Street.num_active_each_dir[oppositeEnd[Street.direction]] != 0)) {
      // set street direction to be opposite
      Street.direction = oppositeEnd[Street.direction];
      VERBOSE_PRINT ("intersection has changed to %s\n", Street.direction==EAST ? "EAST" : "WEST");
      // signal opposite direction can enter
      for (int i = 0; i < MAX_OCCUPANCY; i++) {
        // check that not all EAST cars are finished
        if ((Street.direction == EAST) && (Street.num_active_each_dir[0] != 0)) {
          VERBOSE_PRINT ("signalling EAST can enter\n");
          // signal east can go
          uthread_sem_signal(Street.dir_sems[EAST]);
        }
        // check that not all WEST cars are finished
        else if ((Street.direction == WEST) && (Street.num_active_each_dir[1] != 0)) {
          VERBOSE_PRINT ("signalling WEST can enter\n");
          // signal west can go
          uthread_sem_signal(Street.dir_sems[WEST]);
        }
      }
  // else if intersection is not full, signal
  } else if (Street.count < MAX_OCCUPANCY) {
      if (Street.direction == EAST && Street.num_active_each_dir[0] != 0)
        uthread_sem_signal(Street.dir_sems[EAST]);
      else if (Street.direction == WEST && Street.num_active_each_dir[1] != 0)
        uthread_sem_signal(Street.dir_sems[WEST]);  
  }

  // unlock street
  uthread_sem_signal(Street.lock);  
}

void* car () {
  const enum Direction dir = rand() % 2;

  // update num_active_each_dir array
  uthread_sem_wait(Street.lock);
  if (dir == EAST) Street.num_active_each_dir[0]++;
  else Street.num_active_each_dir[1]++;
  uthread_sem_signal(Street.lock);

  // loop for NUM_ITERATIONS
  for (int i = 0; i < NUM_ITERATIONS; i++) {
    // attempt to enter street
    VERBOSE_PRINT ("car going %s is attempting to enter street\n", dir==EAST ? "EAST" : "WEST");
    enterStreet(dir);
    VERBOSE_PRINT ("car going %s is crossing\n", dir==EAST ? "EAST" : "WEST");
    // yield CROSSING_TIME times
    for (int j = 0; j < CROSSING_TIME; j++)
      uthread_yield();

    VERBOSE_PRINT ("%d EAST cars and %d WEST cars active\n", Street.num_active_each_dir[0], Street.num_active_each_dir[1]); 

    // exit street
    VERBOSE_PRINT ("car going %s is exiting street\n", dir==EAST ? "EAST" : "WEST");
    leaveStreet(dir);

    VERBOSE_PRINT ("car going %s is yielding before attempting to enter street again\n", dir==EAST ? "EAST" : "WEST");
    // yield WAIT_TIME_BETWEEN_CROSSES times
    for (int j = 0; j < WAIT_TIME_BETWEEN_CROSSES; j++)
      uthread_yield();

    VERBOSE_PRINT ("%s car has finished %d iterations\n", dir==0 ? "EAST" : "WEST", i);
  }

  // update num_active_each_dir array
  uthread_sem_wait(Street.lock);
  if (dir == EAST) Street.num_active_each_dir[0]--;
  else Street.num_active_each_dir[1]--;
  VERBOSE_PRINT ("%d EAST cars and %d WEST cars active\n", Street.num_active_each_dir[0], Street.num_active_each_dir[1]);
  uthread_sem_signal(Street.lock);
}

int main (int argc, char** argv) {
  
  uthread_init(8);

  waitingHistogramLock = uthread_sem_create(1);
  entryTicker = 0;

  initializeStreet();
  uthread_t pt [NUM_CARS];

  for (int i = 0; i < NUM_CARS; i++)
    pt[i] = uthread_create(car, NULL);

  for (int i = 0; i < NUM_CARS; i++)
    uthread_join(pt[i], NULL);
  
  printf ("Times with 1 car  going east: %d\n", occupancyHistogram [EAST] [1]);
  printf ("Times with 2 cars going east: %d\n", occupancyHistogram [EAST] [2]);
  printf ("Times with 3 cars going east: %d\n", occupancyHistogram [EAST] [3]);
  printf ("Times with 1 car  going west: %d\n", occupancyHistogram [WEST] [1]);
  printf ("Times with 2 cars going west: %d\n", occupancyHistogram [WEST] [2]);
  printf ("Times with 3 cars going west: %d\n", occupancyHistogram [WEST] [3]);
  
  printf ("Waiting Histogram\n");
  for (int i=0; i < WAITING_HISTOGRAM_SIZE; i++)
    if (waitingHistogram [i])
      printf ("  Cars waited for           %4d car%s to enter: %4d time(s)\n",
	      i, i==1 ? " " : "s", waitingHistogram [i]);
  if (waitingHistogramOverflow)
    printf ("  Cars waited for more than %4d cars to enter: %4d time(s)\n",
	    WAITING_HISTOGRAM_SIZE, waitingHistogramOverflow);
}
