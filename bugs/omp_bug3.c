/******************************************************************************
* DESCRIPTION: tbd
* BUG: Deadlock for OMP_NUM_THREADS>2
* AUTHOR: Blaise Barney  01/09/04
******************************************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#define N 50

int main(int argc, char *argv[])
{
  int i, nthreads, tid, section;
  float a[N], b[N], c[N];
  void print_results(float array[N], int tid, int section);

  /* Some initializations */
  for (i = 0; i < N; i++)
    a[i] = b[i] = (float)i;

#pragma omp parallel private(c, i, tid, section)
  {
    tid = omp_get_thread_num();
    if (tid == 0)
    {
      nthreads = omp_get_num_threads();
      printf("Number of threads = %d\n", nthreads);
    }

/*** Use barriers for clean output ***/
#pragma omp barrier
    printf("Thread %d starting...\n", tid);
#pragma omp barrier

#pragma omp sections nowait // nowait is great if you want some postprocessing later or something like that.
    {
#pragma omp section
      {
        tid = omp_get_thread_num();
        printf("THREAD %d\n", tid);
        section = 1;
        for (i = 0; i < N; i++)
          c[i] = a[i] * b[i];
        print_results(c, tid, section);
      }

#pragma omp section
      {
        tid = omp_get_thread_num();
        printf("THREAD %d\n", tid);
        section = 2;
        for (i = 0; i < N; i++)
          c[i] = a[i] + b[i];
        print_results(c, tid, section);
      }

    } /* end of sections */

/*** Use barrier for clean output ***/
#pragma omp barrier
    printf("Thread %d exiting...\n", tid);

  } /* end of parallel section */
}

void print_results(float array[N], int tid, int section)
{
  int i, j;

  j = 1;
/*** use critical for clean output ***/
#pragma omp critical
  {
    printf("\nThread %d did section %d. The results are:\n", tid, section);
    for (i = 0; i < N; i++)
    {
      printf("%e  ", array[i]);
      j++;
      if (j == 6)
      {
        printf("\n");
        j = 1;
      }
    }
    printf("\n");
  } /*** end of critical ***/

#pragma omp barrier  // problem is that threads 0 and 1 will wait for all other threads in the same parallel construct, but they will never get into this line
  printf("Thread %d done and synchronized.\n", tid);
}
