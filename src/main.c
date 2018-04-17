#include "../include/parmetis.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

const char* prePath = "/mnt/nfs/zpltys/tempDir/";
//const int vertexNum = 32768;
const int vertexNum = 16;
const int PART = 2;

struct Edge {
    idx_t x, y;
};
typedef struct Edge edge;

int cmp(const void* l, const void* r) {
    edge *a, *b;
    a = (edge*)l;
    b = (edge*)r;
    if (a->x == b->x) {
        return a->y - b->y;
    } else {
        return a->x - b->x;
    }
}

int main (int argc, char *argv[]) {
    int myid, numprocs, namelen;
    int i, j, n;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);        /* starts MPI */
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);  /* get current process id */
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);      /* get number of processes */
    MPI_Get_processor_name(processor_name, &namelen);


    char *path = (char*) malloc((strlen(prePath) + 20) * sizeof(char));
    sprintf(path, "%sG.%d", prePath, myid);
    FILE *fp = fopen(path, "r");

    idx_t x, y;
    edge* e;
    e = (edge*)(malloc(1000 * sizeof(edge)));
    i = 0;
    while (~fscanf(fp, "%d%d", &x, &y)) {
        e[i].x = x;
        e[i].y = y;
        i++;
    }
    //printf("i:%d\n", i);
    qsort(e, i, sizeof(edge), cmp);
    n = i;

   // delete[] path;

    idx_t *vtxdist, *xadj, *adjcny, wgtflag, numflag, ncon, nparts, *options, edgecut, *part;
    MPI_Comm comm;
    real_t *tpwgts, *ubvec;
    vtxdist = (idx_t*)malloc((numprocs + 3) * sizeof(idx_t));
    xadj = (idx_t*)malloc((vertexNum / numprocs + 3) * sizeof(idx_t));
    adjcny = (idx_t*)malloc((n + 3) * sizeof(idx_t));
    for (i = 0; i <= numprocs; i++) vtxdist[i] = vertexNum / numprocs * i;

    idx_t bx = vtxdist[myid];
    i = 0;
    j = 0;
    int k = 0;
    xadj[j++] = 0;
    edge *it;
    for (k = 0; k < n; k++) {
        it = e + k;
        printf("x:%d y:%d\n", it->x, it->y);
        x = it->x;
        y = it->y;
        if (bx == x) {
            if (y == adjcny[i - 1]) continue;
            adjcny[i++] = y;
        } else {
            while (bx < x) {
                bx++;
                xadj[j++] = i;
            }
            adjcny[i++] = y;
        }
    }
    while (bx < vtxdist[myid + 1]) {
        bx++;
        xadj[j++] = i;
    }

    wgtflag = 0;
    numflag = 0;
    ncon = 1;
    nparts = PART;

    //tpwgts = new real_t[1 * numprocs + 10];
    tpwgts = (real_t*)malloc(1 * (numprocs + 10) * sizeof(real_t));
    ubvec = (real_t*)malloc((numprocs + 10) * sizeof(real_t));
    for (i = 0; i < 1 * numprocs; i++) {
        tpwgts[i] = 1.0 / numprocs;
        ubvec[i] = 1.05;
    }
    options = (idx_t*)malloc(5 * sizeof(idx_t));
    options[0] = 0;
    edgecut = 0;
    part = (idx_t*)malloc((vertexNum / numprocs + 10) * sizeof(idx_t));
    part[vertexNum / numprocs - 1] = 1;
    comm = MPI_COMM_WORLD;

    printf("process %d run func!\n", myid);

    xadj = {0,1,1,3,3,4,4,4,4,6,6,6,6,8,8,8,8};
    adjcny = {0,1,1,3,3,4,4,4};

    printf("xadj:\n");
    for (i = 0; i <= vertexNum / numprocs; i++) {
        printf("%d ", xadj[i]);
    }
    printf("\nadjcny:\n");
    for (i = 0; i < vertexNum / numprocs; i++) {
        for (j = xadj[i]; j < xadj[i + 1]; j++) {
            printf("%d ", adjcny[j]);
        }
        printf("\n");
    }
    printf("vtxdist:");
    for (i = 0; i <= numprocs; i++) {
        printf("%d ", vtxdist[i]);
    }
    printf("\n");

    ParMETIS_V3_PartKway(vtxdist, xadj, adjcny, NULL, NULL, &wgtflag, &numflag, &ncon, &nparts, tpwgts, ubvec, options,
                         &edgecut, part, &comm);
    //for (i = 0; i <= numprocs; i++) vtxdist[i] = vertexNum / numprocs * i;
    for (i = 0; i <= vertexNum / numprocs; i++) {
        printf("part %d to %d\n", vtxdist[myid] + i, part[i]);
    }

    MPI_Finalize();
    return 0;
}
