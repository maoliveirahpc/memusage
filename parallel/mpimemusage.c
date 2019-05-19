/*=====================================================================================*/
/*= MPI Code to launch an application and report memory usage                         =*/
/*=====================================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char** argv)
{
int rank, size, namelen, version, subversion, universe_size;
MPI_Comm family_comm;
char processor_name[MPI_MAX_PROCESSOR_NAME], worker_program[30];
char **c_arg;

MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);
/* We are only the managers and we must be only one! */
if (size != 1)
{
  printf("==>Error: Only one manager process should be running, but %d were started.\n", size);
  MPI_Finalize();
  return 1;
}
/* Check we have all we need */
if ( argc != 2 )
{
	printf("Use as:\n");
	printf("\t mpiexec.1ppj mpimemusage executable_to_run\n");
	printf("\n");
        MPI_Finalize();
        return 1;
}
/* Some info */
MPI_Get_processor_name(processor_name, &namelen);
MPI_Get_version(&version, &subversion);
printf("==>Process manager on %s running MPI %d.%d\n", processor_name, version, subversion);
/* Get universe size and we cannot be zero */
universe_size=atoi(getenv("MPIPROCS"));
if (!universe_size)
{
	printf("==>Error: Universe size is zero!");
	MPI_Finalize();
	return 0;
}
/* The serial memory usager code */
strcpy(worker_program, "/home/mao/memusage/serial/memusage");
/* What we really want to run */
c_arg=(char **)malloc(2*sizeof(char *));
(*(c_arg))=argv[1];
(*(c_arg+1))=NULL;
printf("==>Spawning %d worker processes running %s %s\n", universe_size,worker_program,argv[1]);
MPI_Comm_spawn(worker_program, c_arg, universe_size, MPI_INFO_NULL, 0, MPI_COMM_SELF, &family_comm, MPI_ERRCODES_IGNORE);
free(c_arg);
MPI_Finalize();
return 0;
}
