//
// Written by Mike Feeley, University of BC, 2010
// Do not redistribute or otherwise make available any portion of this code to anyone without written permission of the author.
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "uthread.h"

char* usage = "usage: sor N threads iterations processors";

struct square_matrix_t {
  int  size;
  int* value;
};

void matrix_set (struct square_matrix_t* matrix, int row, int col, int value) {
  row = (row + matrix->size) % matrix->size;
  col = (col + matrix->size) % matrix->size;
  matrix->value [row*matrix->size + col] = value;
}

int matrix_get (struct square_matrix_t* matrix, int row, int col) {
  row = (row + matrix->size) % matrix->size;
  col = (col + matrix->size) % matrix->size;
  return matrix->value [row*matrix->size + col];
}

struct square_matrix_t* matrix_create (int size) {
  struct square_matrix_t* matrix;
  
  matrix        = (struct square_matrix_t *) malloc (sizeof (struct square_matrix_t));
  matrix->size  = size;
  matrix->value = (int *) malloc (sizeof (int) * size * size);
  return matrix;
}

void matrix_set_initial_values (struct square_matrix_t* matrix) {
  int row, col;
  
  for (row=0; row<matrix->size; row++)
    for (col=0; col<matrix->size; col++)
      matrix_set (matrix, row, col, row*1000 + col*10);
}

void matrix_copy (struct square_matrix_t* from, struct square_matrix_t* to,
                  int start_row, int end_row, int start_col, int end_col)
{
  int row, col;
  
  for (row=start_row; row<=end_row; row++)
    for (col=start_col; col<=end_col; col++)
      matrix_set (to, row, col, matrix_get (from, row, col));
}

void matrix_print (struct square_matrix_t* matrix) {
  int row, col;
  
  for (row=0; row<matrix->size; row++) {
    for (col=0; col<matrix->size; col++) {
      printf ("%8d",matrix_get(matrix,row,col));
    }
    printf ("\n");
  }
}

int matrix_compare (struct square_matrix_t* a, struct square_matrix_t* b) {
  int row, col;
  
  if (a->size != b->size)
    return 0;
  else {
    for (row=0; row<a->size; row++)
      for (col=0; col<a->size; col++)
        if (matrix_get (a,row,col) != matrix_get (b,row,col))
          return 0;
  }
  return 1;
}

void relax_one_step (struct square_matrix_t* in_matrix, struct square_matrix_t* out_matrix,
                     int start_row, int end_row, int start_col, int end_col) 
{
  int out_row, out_col, out_total, in_row, in_col;
  
  for (out_row=start_row; out_row<=end_row; out_row++)
    for (out_col=start_col; out_col<=end_col; out_col++) {
      out_total = 0;
      for (in_row=out_row-1; in_row<=out_row+1; in_row++)
        for (in_col=out_col-1; in_col<=out_col+1; in_col++) 
          out_total += matrix_get (in_matrix, in_row, in_col);
      matrix_set (out_matrix, out_row, out_col, out_total / 9);
    }
}

struct relax_args {
  struct square_matrix_t*   matrix;
  struct square_matrix_t*   temp_matrix;
  int                       start_row, end_row, start_col, end_col;
  int                       iterations;
  uthread_barrier_t*        barrier;
};

void* relax (void* args_no_type) {
  struct relax_args* args = args_no_type;
  int i;
  
  for (i=0; i<args->iterations; i++) {
    relax_one_step (args->matrix, args->temp_matrix, args->start_row, args->end_row, args->start_col, args->end_col);
    uthread_barrier_arrive (args->barrier);
    matrix_copy (args->temp_matrix, args->matrix, args->start_row, args->end_row, args->start_col, args->end_col);
    uthread_barrier_arrive (args->barrier);
  }
  return 0;
}

void sor (int N, int tpr, int I) {
  struct relax_args* args;
  struct square_matrix_t *matrix, *temp_matrix, *control_matrix;
  int i,j;
  uthread_t** t;
  uthread_barrier_t* barrier;
  
  printf ("SOR on %d by %d matrix, using %d threads, for %d iterations\n",N,N,tpr*tpr,I);
  matrix         = matrix_create (N);
  temp_matrix    = matrix_create (N);
  control_matrix = matrix_create (N);
  matrix_set_initial_values (matrix);

#ifdef PRINT
  printf ("initial matrix:\n");
  matrix_print (matrix);
#endif
  
#ifdef TEST
  // single-threaded control case
  barrier = uthread_barrier_create (1);
  matrix_copy (matrix, control_matrix, 0, N-1, 0, N-1);
  args              = (struct relax_args*) malloc (sizeof (struct relax_args));
  args->matrix      = control_matrix;
  args->temp_matrix = temp_matrix;
  args->start_row   = 0;
  args->end_row     = N - 1;
  args->start_col   = 0;
  args->end_col     = N-1;
  args->iterations  = I;
  args->barrier     = barrier;
  relax (args);
#endif
  
  // multi-threaded case
  barrier = uthread_barrier_create (tpr*tpr);
  t       = (uthread_t**) malloc (sizeof (uthread_t*) * tpr * tpr);
  for (i=0; i<tpr; i++)
    for (j=0; j<tpr; j++) {
      args             = (struct relax_args*) malloc (sizeof (struct relax_args));
      args->matrix      = matrix;
      args->temp_matrix = temp_matrix;
      args->start_row   = i * N/tpr;
      args->end_row     = (i+1) * N/tpr - 1;
      args->start_col   = j * N/tpr;
      args->end_col     = (j+1) * N/tpr - 1;
      args->iterations  = I;
      args->barrier     = barrier;
      t[i*tpr+j] = uthread_create (relax, args);
    }
  for (i=0; i<tpr; i++)
    for (j=0; j<tpr; j++)
      uthread_join (t[i*tpr+j]);

#ifdef PRINT
  printf ("relaxed matrix:\n");
  matrix_print (control_matrix);
#endif
#ifdef TEST
  if (matrix_compare (matrix, control_matrix)) 
    printf ("Multi-threaded execution produces same results as single-threaded\n");
  else 
    printf ("Multi-threaded output error\n");
#endif
}

int main (int argc, char** argv) {
  char *N_end, *T_end, *I_end, *P_end;
  int   N, T, I, P, tpr;
  if (argc == 5) {
    N   = strtol (argv[1], &N_end, 0);
    T   = strtol (argv[2], &T_end, 0);
    I   = strtol (argv[3], &I_end, 0);
    P   = strtol (argv[4], &P_end, 0);
    tpr = T>1? sqrt (T): 1;
  }
  if (argc != 5 || *N_end!=0 || *T_end!=0 || *I_end!=0 || *P_end!=0 || tpr*tpr != T || N % tpr != 0) {
    puts (usage);
    exit (EXIT_FAILURE);
  }
  uthread_init (P);
  sor          (N,tpr,I);
}