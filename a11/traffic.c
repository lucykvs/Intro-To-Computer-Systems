#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include "uthread.h"
#include "uthread_mutex_cond.h"

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
  uthread_mutex_t lock;
  enum Direction direction;   // current direction of cars in street
  uthread_cond_t east;   // to signal that eastbound cars can enter
  uthread_cond_t west;   // to signal that westbound cars can enter
  int count; 
  int num_each_dir[2];    // keeps track of number of (active) threads in each direction (num EAST, num WEST)
} Street;

void initializeStreet(void) {
  Street.lock = uthread_mutex_create();
  Street.direction = 0;
  Street.east = uthread_cond_create(Street.lock);
  Street.west = uthread_cond_create(Street.lock);
  Street.count = 0;
  Street.num_each_dir[0] = 0;
  Street.num_each_dir[1] = 0;
}

#define WAITING_HISTOGRAM_SIZE (NUM_ITERATIONS * NUM_CARS)
int             entryTicker;                           // incremented with each entry
int             waitingHistogram [WAITING_HISTOGRAM_SIZE];
int             waitingHistogramOverflow;
uthread_mutex_t waitingHistogramLock;
int             occupancyHistogram [2] [MAX_OCCUPANCY + 1];



void recordWaitingTime (int waitingTime) {
  uthread_mutex_lock (waitingHistogramLock);
  if (waitingTime < WAITING_HISTOGRAM_SIZE)
    waitingHistogram [waitingTime] ++;
  else
    waitingHistogramOverflow ++;
  uthread_mutex_unlock (waitingHistogramLock);
}



void enterStreet (enum Direction g) {
  // lock street as critical section
  uthread_mutex_lock(Street.lock);

  // if intersection is empty, set direction to car trying to enter
  if (Street.count == 0) {
    // check that not all cars in direction g are finished
    if (Street.num_each_dir[g] != 0) {
      Street.direction = g;
      VERBOSE_PRINT ("intersection has changed to %s\n", g==EAST ? "EAST" : "WEST");
    }
  }
  
  // record time before entering and/or waiting
  int t = entryTicker;
  
  // if intersection not empty and car going opposite direction, or intersection is full, wait for relevant signal
  while (g != Street.direction || Street.count >= MAX_OCCUPANCY) {
    if (g == EAST) {
      VERBOSE_PRINT ("car going EAST is waiting to enter street\n");
      uthread_cond_wait(Street.east);
    }
    else {
      VERBOSE_PRINT ("car going WEST is waiting to enter street\n"); 
      uthread_cond_wait(Street.west);
    }
  }

  // calculate waiting time to enter street
  int wait_time = entryTicker - t;
  // add to waitingHistogram
  recordWaitingTime(wait_time);

  // enter street
  VERBOSE_PRINT ("%s-bound car has entered street\n", g==EAST ? "EAST" : "WEST");
  Street.count++; 
  // increment entry time
  entryTicker++;
  // add to occupancyHistogram
  occupancyHistogram[g][Street.count]++;

  uthread_mutex_unlock(Street.lock);
}



void leaveStreet(void) {
  // lock street as critical section
  uthread_mutex_lock(Street.lock);
  // decrement num cars in street
  Street.count--;

  // if street is empty and not all cars in opposite direction are finished
  if ((Street.count == 0) && (Street.num_each_dir[oppositeEnd[Street.direction]] != 0)) {
      // set street direction to be opposite
      Street.direction = oppositeEnd[Street.direction];
      VERBOSE_PRINT ("intersection has changed to %s\n", Street.direction==EAST ? "EAST" : "WEST");
      // signal opposite direction can enter
      for (int i = 0; i < MAX_OCCUPANCY; i++) {
        // check that not all EAST cars are finished
        if ((Street.direction == EAST) && (Street.num_each_dir[0] != 0)) {
          VERBOSE_PRINT ("signalling EAST can enter\n");
          // signal east can go
          uthread_cond_signal(Street.east);
        }
        // check that not all WEST cars are finished
        else if ((Street.direction == WEST) && (Street.num_each_dir[1] != 0)) {
          VERBOSE_PRINT ("signalling WEST can enter\n");
          // signal west can go
          uthread_cond_signal(Street.west);
        }
      }
  // else if intersection is not full, signal
  } else if (Street.count < MAX_OCCUPANCY) {
      if (Street.direction == EAST && Street.num_each_dir[0] != 0)
        uthread_cond_signal(Street.east);
      else if (Street.direction == WEST && Street.num_each_dir[1] != 0)
        uthread_cond_signal(Street.west);  
  }

  // unlock street
  uthread_mutex_unlock(Street.lock);  
}



void* car () {
  const enum Direction dir = rand() % 2;

  // update num_each_dir array
  uthread_mutex_lock(Street.lock);
  if (dir == EAST) Street.num_each_dir[0]++;
  else Street.num_each_dir[1]++;
  uthread_mutex_unlock(Street.lock);

  // loop for NUM_ITERATIONS
  for (int i = 0; i < NUM_ITERATIONS; i++) {
    // attempt to enter street
    VERBOSE_PRINT ("car going %s is attempting to enter street\n", dir==EAST ? "EAST" : "WEST");
    enterStreet(dir);
    VERBOSE_PRINT ("car going %s is crossing\n", dir==EAST ? "EAST" : "WEST");
    // yield CROSSING_TIME times
    for (int j = 0; j < CROSSING_TIME; j++)
      uthread_yield();
      
    // exit street
    VERBOSE_PRINT ("car going %s is exiting street\n", dir==EAST ? "EAST" : "WEST");
    leaveStreet();

    VERBOSE_PRINT ("car going %s is yielding before attempting to enter street again\n", dir==EAST ? "EAST" : "WEST");
    // yield WAIT_TIME_BETWEEN_CROSSES times
    for (int j = 0; j < WAIT_TIME_BETWEEN_CROSSES; j++)
      uthread_yield();
  }

  // update num_each_dir array
  uthread_mutex_lock(Street.lock);
  if (dir == EAST) Street.num_each_dir[0]--;
  else Street.num_each_dir[1]--;
  VERBOSE_PRINT ("%d EAST cars and %d WEST cars active\n", Street.num_each_dir[0], Street.num_each_dir[1]);
  uthread_mutex_unlock(Street.lock);
}



int main (int argc, char** argv) {

  uthread_init(8);

  waitingHistogramLock = uthread_mutex_create();
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
