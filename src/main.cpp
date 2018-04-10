#include "../include/parmetis.h"
#include <cstdio>

int main (int argc, char *argv[])
//      int argc;
//      char *argv[];
{
    int myid, numprocs, namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init (&argc, &argv);        /* starts MPI */
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);  /* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);      /* get number of processes */
    MPI_Get_processor_name(processor_name,&namelen);

    if(myid == 0) printf("number of processes: %d\n",numprocs);
    printf( "%s: Hello world from process %d \n", processor_name, myid);

    if (myid == 0) {
        idx_t a = 0;
        printf("len of a is %d\n", sizeof(a));
        long long b = 0;
        printf("len of b is %d\n", sizeof(b));
    }
    MPI_Finalize();
    return 0;
}