#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  int numtasks, rank, tag = 1, alpha, i;
  float beta;
  MPI_Request reqs[10];
  MPI_Status stats[10];

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0)
  {
    if (numtasks > 2)
      printf("Numtasks=%d. Only 2 needed. Ignoring extra...\n", numtasks);
    for (i = 0; i < 10; i++)
    {
      alpha = i * 10;
      MPI_Isend(&alpha, 1, MPI_INT, 1, tag, MPI_COMM_WORLD, &reqs[i]); // tag = 1, send to 1
      // ISend means you cannot reuse send buffer. It doesn't imply anything on the performance of such a code. Returns immediately to the user.
      MPI_Wait(&reqs[i], &stats[i]);  // this is basically blocking code
      printf("Task %d sent = %d\n", rank, alpha);
    }
  }

  if (rank == 1)
  {
    for (i = 0; i < 10; i++)
    {
      MPI_Irecv(&beta, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &reqs[i]); // non-blocking code with tag = 1, receive from 0
      MPI_Wait(&reqs[i], &stats[i]);
      printf("Task %d received = %f\n", rank, beta);
    }
  }

  MPI_Finalize();
}
