/******************************************************************************
* DESCRIPTION: Parallel integer summation
* BUG: Wrong result
* AUTHOR: Blaise Barney 
******************************************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  int nthreads, i, tid;
  long long total;
  const long long N = 1000000;

/*** Spawn parallel region ***/
#pragma omp parallel
  {
    /* Obtain thread number */
    tid = omp_get_thread_num();
    /* Only master thread does this */
    if (tid == 0)
    {
      nthreads = omp_get_num_threads();
      printf("Number of threads = %d\n", nthreads);
    }
    printf("Thread %d is starting...\n", tid);

#pragma omp barrier

    /* do some work */
    total = 0;
#pragma omp for schedule(dynamic, 10)
    for (i = 0; i < N; i++)
      total += i;

    printf("Thread %d is done!\n", tid);

  } /*** End of parallel region ***/
  printf("Total = %lld\n", total);
  printf("Expected result: Total = %lld\n", ((N - 1) * (N - 1) + (N - 1)) / 2);
}
