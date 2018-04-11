#include "../include/parmetis.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>
#include <vector>
using namespace std;

char* prePath = "/mnt/nfs/zpltys/tempDir/";

struct edge {
    idx_t x, y;
};

bool cmp(const edge& a, const edge& b) {
    if (a.x == b.x) {
        return a.y < b.y;
    } else {
        return a.x < b.x;
    }
}

int main (int argc, char *argv[])
{
    int myid, numprocs, namelen;
    int i;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init (&argc, &argv);        /* starts MPI */
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);  /* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);      /* get number of processes */
    MPI_Get_processor_name(processor_name,&namelen);



    char *path = new char[strlen(prePath) + 20];
    sprintf(path, "%sG.%d", prePath, myid);
    FILE* fp = fopen(path, "r");

    idx_t x, y;
    vector<edge> e;
    while(~fscanf(fp, "%d%d", &x, &y)){
        edge tmp;
        tmp.x = x; tmp.y = y;
        e.push_back(tmp);
    }
    sort(e.begin(), e.end(), cmp);

    if(myid == 0) {
        for (vector<edge>::iterator it = e.begin(); it != e.end(); it++) {
            printf("%d %d\n", it->x, it->y);
        }
    }
    delete[] path;
    MPI_Finalize();
    return 0;
}