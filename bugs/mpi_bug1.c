#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  int numtasks, rank, dest, tag, source, rc, count;
  char inmsg, outmsg = 'x';
  MPI_Status Stat;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Task %d starting...\n", rank);

  if (rank == 0)
  {
    if (numtasks > 2)
      printf("Numtasks=%d. Only 2 needed. Ignoring extra...\n", numtasks);
    dest = rank + 1;
    source = dest;
    tag = 0;
    rc = MPI_Send(&outmsg, 1, MPI_CHAR, dest, tag, MPI_COMM_WORLD); // 0 sends to 1 with tag 0
    printf("Sent to task %d...\n", dest);
    rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat); // 0 receives from 1 with tag 0
    printf("Received from task %d...\n", source);
  }

  else if (rank == 1)
  {
    dest = rank - 1;
    source = dest;
    tag = 0;
    rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat); // receive from 0 with tag 1
    printf("Received from task %d...\n", source); 
    rc = MPI_Send(&outmsg, 1, MPI_CHAR, dest, tag, MPI_COMM_WORLD); // send from 0 with tag 1
    printf("Sent to task %d...\n", dest);
  }

  if (rank < 2)
  {
    rc = MPI_Get_count(&Stat, MPI_CHAR, &count); // how many you obtained
    printf("Task %d: Received %d char(s) from task %d with tag %d \n",
           rank, count, Stat.MPI_SOURCE, Stat.MPI_TAG);
  }

  MPI_Finalize();
}
