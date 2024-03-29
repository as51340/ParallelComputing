/******************************************************************************
* DESCRIPTION:
*   Using SECTIONS, two threads initialize their own array and then add
*   it to the other's array
* BUG: Deadlock
* AUTHOR: Blaise Barney  01/29/04
******************************************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#define N 1000000
#define PI 3.1415926535
#define DELTA .01415926535

int main(int argc, char *argv[])
{
  int nthreads, tid, i;
  float a[N], b[N];
  omp_lock_t locka, lockb;

  /* Initialize the locks */
  omp_init_lock(&locka);
  omp_init_lock(&lockb);

  /* Initialize the arrays */
  for (i = 0; i < N; i++)
  {
    a[i] = 0;
    b[i] = 0;
  }

/* Fork a team of threads giving them their own copies of variables */
#pragma omp parallel shared(a, b, nthreads, locka, lockb) private(tid, i)
  {

    /* Obtain thread number and number of threads */
    tid = omp_get_thread_num();
#pragma omp master  // only master can execute this section
    {
      nthreads = omp_get_num_threads();
      printf("Number of threads = %d\n", nthreads);
    }
    printf("Thread %d starting...\n", tid);
#pragma omp barrier // all threads within parallel region must reach this point

#pragma omp sections nowait  // threads may continue to do its work
    {
#pragma omp section
      {
        omp_set_lock(&locka);
        printf("Thread %d updating a[]\n", tid);
        for (i = 0; i < N; i++)
          a[i] += DELTA * i;
        omp_unset_lock(&locka);
        omp_set_lock(&lockb);
        printf("Thread %d updating b[]\n", tid);
        for (i = 0; i < N; i++)
          b[i] += DELTA + i;
      }

#pragma omp section
      {
        omp_set_lock(&lockb);
        printf("Thread %d updating b[]\n", tid);
        for (i = 0; i < N; i++)
          b[i] += PI * i;
        omp_unset_lock(&lockb);
        omp_set_lock(&locka);
        printf("Thread %d updating a[]\n", tid);
        for (i = 0; i < N; i++)
          a[i] += PI + i;
        omp_unset_lock(&locka);
      }

    } /* end of sections */
  }   /* end of parallel region */

  printf("Sample results: %f %f %f %f\n", a[0], b[0], a[N - 1], b[N - 1]);
}
