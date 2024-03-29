#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#define ARRAYSIZE 16000000
#define MASTER 0

long data[ARRAYSIZE];

int main(int argc, char *argv[])
{
  int numtasks, taskid, rc, dest, offset, i, j, tag1,
      tag2, source, chunksize;
  long mysum, sum;
  long update(int myoffset, int chunk, int myid);
  MPI_Status status;

  /***** Initializations *****/
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
  if (numtasks % 4 != 0)
  {
    printf("Quitting. Number of MPI tasks must be divisible by 4.\n");
    MPI_Abort(MPI_COMM_WORLD, rc);
    exit(0);
  }
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid); // MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
  printf("MPI task %d has started...\n", taskid);
  chunksize = (ARRAYSIZE / numtasks);
  tag2 = 1;
  tag1 = 2;

  /***** Master task only ******/
  if (taskid == MASTER)
  {

    /* Initialize the array */
    sum = 0;
    for (i = 0; i < ARRAYSIZE; i++)
    {
      data[i] = i * 1.0;
      sum = sum + data[i];
    }
    printf("Initialized array sum = %ld\n", sum);

    /* Send each task its portion of the array - master keeps 1st part */
    offset = chunksize;
    for (dest = 1; dest < numtasks; dest++)
    {
      MPI_Send(&offset, 1, MPI_INT, dest, tag1, MPI_COMM_WORLD); //send with tag1 - blocking send - will not return until it is not safe to reuse buffer - it may buffer at sender's or receiver's end
      // or wait for corresponding receive to match
      MPI_Send(&data[offset], chunksize, MPI_FLOAT, dest, tag2, MPI_COMM_WORLD); // send with tag2 real data with offset. this looks fine. It would be better to use scatter operation. But then all processors need to write this same command.

      printf("Sent %d elements to task %d offset= %d\n", chunksize, dest, offset);
      offset = offset + chunksize;
    }

    /* Master does its part of the work */
    offset = 0;
    mysum = update(offset, chunksize, taskid);

    /* Wait to receive results from each task */
    for (i = 1; i < numtasks; i++)
    {
      source = i;
      MPI_Recv(&offset, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status); // receive offset from each processor
      MPI_Recv(&data[offset], chunksize, MPI_LONG, source, tag2,
               MPI_COMM_WORLD, &status); // okay same but why do I need that?
    }

    /* Get final sum and print sample results */
    MPI_Reduce(&mysum, &sum, 1, MPI_LONG, MPI_SUM, MASTER, MPI_COMM_WORLD);
    printf("Sample results: \n");
    offset = 0;
    for (i = 0; i < numtasks; i++)
    {
      for (j = 0; j < 5; j++)
        printf("  %ld", data[offset + j]);
      printf("\n");
      offset = offset + chunksize;
    }
    printf("*** Final sum= %ld ***\n", sum);
    printf("(This should be twice the inital sum.)\n");

  } /* end of master section */

  /***** Non-master tasks only *****/

  if (taskid > MASTER)
  {

    /* Receive my portion of array from the master task */
    source = MASTER;
    MPI_Recv(&offset, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
    MPI_Recv(&data[offset], chunksize, MPI_FLOAT, source, tag2,
             MPI_COMM_WORLD, &status); // this accepting also looks very good 

    mysum = update(offset, chunksize, taskid);

    /* Send my results back to the master task */
    dest = MASTER;
    MPI_Send(&offset, 1, MPI_INT, dest, tag1, MPI_COMM_WORLD); // send is ok with tag1
    MPI_Send(&data[offset], chunksize, MPI_LONG, MASTER, tag2, MPI_COMM_WORLD); // send okay with tag 2

    MPI_Reduce(&mysum, &sum, 1, MPI_LONG, MPI_SUM, MASTER, MPI_COMM_WORLD);

  } /* end of non-master */


  // OR if you want to be smarter

  // MPI_Scatter((&data[0], chunksize, MPI_FLOAT, &local_data[0], chunksize, MPI_FLOAT , MASTER, MPI_COMM_WORLD); 

  MPI_Finalize();

} /* end of main */

long update(int myoffset, int chunk, int myid)
{
  int i;
  long mysum;
  /* Perform addition to each of my array elements and keep my sum */
  mysum = 0;
  for (i = myoffset; i < myoffset + chunk; i++)
  {
    data[i] = data[i] + i * 1.0;
    mysum = mysum + data[i];
  }
  printf("Task %d mysum = %ld\n", myid, mysum);
  return (mysum);
}
