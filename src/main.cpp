#include "../include/parmetis.h"
#include <cstdio>
#include <cstring>
#include <string>

char* prePath = "/mnt/nfs/zpltys/tempDir/";
int main (int argc, char *argv[])
{
    int myid, numprocs, namelen;
    int i;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init (&argc, &argv);        /* starts MPI */
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);  /* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);      /* get number of processes */
    MPI_Get_processor_name(processor_name,&namelen);

    if(myid == 0) printf("number of processes: %d\n",numprocs);
    printf( "%s: Hello world from process %d \n", processor_name, myid);

    char *path = new char[120];
    sprintf(path, "%sG.%d", prePath, myid);
    FILE* fp = fopen(path, "r");

    idx_t x, y;
    while(~fscanf(fp, "%d%d", &x, &y)){
        printf("x:%d y:%d\n", x, y);
    }
    delete[] path;
    MPI_Finalize();
    return 0;
}