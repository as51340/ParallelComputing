#include<omp.h>
#include<stdio.h>
#include<stdlib.h>

#define N 50

int main(int argc, char *argv[]) {

int i = 1;
int n = 5;  // little bit of overhead

#pragma omp parallel private(i) shared(n)
    {
#pragma omp sections
        {
#pragma omp section
            { 
                printf ("id = %d,%d \n", omp_get_thread_num(), i);
            }

#pragma omp section
            { 
                printf ("id = %d,%d \n", omp_get_thread_num(), n);
            }
        }

#pragma omp sections
        {
#pragma omp section
            { 
                printf ("id = %d,%d \n", omp_get_thread_num(), i);
            }

#pragma omp section
            { 
                printf ("id = %d,%d \n", omp_get_thread_num(), n);
            }
        }

    }

    return 0;
}


