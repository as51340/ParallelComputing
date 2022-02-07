/******************************************************************************
* DESCRIPTION:
* BUG: Does not compile (e.g. with gcc 10.2.0)
* AUTHOR: Blaise Barney  6/05
******************************************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#define VECLEN 100

double dotprod(double a[], double b[])
{
  int i, tid;
  double sum;

#pragma omp parallel for reduction(+:sum) private(tid)
  for (i = 0; i < VECLEN; i++)
  {
    tid = omp_get_thread_num();
    sum = sum + (a[i] * b[i]);
    printf("  tid= %d i=%d\n", tid, i);
  }
  return sum;
}

int main(int argc, char *argv[])
{
  int i;
  double sum;
  double a[VECLEN], b[VECLEN];

  for (i = 0; i < VECLEN; i++)
    a[i] = b[i] = (double)i;

  sum = dotprod(a, b);

  printf("Sum = %f\n", sum);
}
