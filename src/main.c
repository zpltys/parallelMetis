#include "../include/parmetis.h"
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

const char* prePath = "/mnt/nfs/zpltys/tempDir/";
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

int main(int argc, char *argv[])
{
    idx_t mype, npes;
    MPI_Comm comm;

    MPI_Init(&argc, &argv);
    MPI_Comm_dup(MPI_COMM_WORLD, &comm);
    MPI_Comm_size(comm, &npes);
    MPI_Comm_rank(comm, &mype);

    if (argc != 2 && argc != 3) {
        if (mype == 0)
            printf("Usage: %s <graph-file> [coord-file]\n", argv[0]);

        MPI_Finalize();
        exit(0);
    }

    TestParMetis_GPart(argv[1], (argc == 3 ? argv[2] : NULL), comm);

    MPI_Comm_free(&comm);

    MPI_Finalize();

    return 0;
}



/***********************************************************************************/
/*! This function tests the various graph partitioning and ordering routines */
/***********************************************************************************/
void TestParMetis_GPart(char *filename, char *xyzfile, MPI_Comm comm) {
    idx_t ncon, nparts, npes, mype, opt2, realcut;
    graph_t graph, mgraph;
    idx_t *part, *mpart, *savepart, *order, *sizes;
    idx_t numflag = 0, wgtflag = 0, options[10], edgecut, ndims;
    real_t ipc2redist, *xyz = NULL, *tpwgts = NULL, ubvec[MAXNCON];

    int myid, numprocs, namelen;

    MPI_Comm_size(comm, &npes);
    MPI_Comm_rank(comm, &mype);

    MPI_Comm_rank(MPI_COMM_WORLD, &myid);  /* get current process id */
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);      /* get number of processes */

    ParallelReadGraph(&graph, filename, comm);
    MPI_Barrier(comm);

    part = imalloc(graph.nvtxs, "TestParMetis_V3: part");
    tpwgts = rmalloc(MAXNCON * npes * 2, "TestParMetis_V3: tpwgts");
    rset(MAXNCON, 1.05, ubvec);

    graph.vwgt = ismalloc(graph.nvtxs * 5, 1, "TestParMetis_GPart: vwgt");


    /*======================================================================
     *     / ParMETIS_V3_PartKway
     *         /=======================================================================*/
    options[0] = 1;
    options[1] = 3;
    options[2] = 1;
    wgtflag = 0;
    numflag = 0;
    edgecut = 0;

    nparts = 2;
    ncon = 1;
    if (ncon > 1 && nparts > 1)
        Mc_AdaptGraph(&graph, part, ncon, nparts, comm);
    else
        iset(graph.nvtxs, 1, graph.vwgt);

    if (mype == 0)
        printf("\nTesting ParMETIS_V3_PartKway with ncon: %"PRIDX", nparts: %"PRIDX"\n", ncon, nparts);

    rset(nparts * ncon, 1.0 / (real_t) nparts, tpwgts);

    int i, n, j;
    printf("vtxdist:\n");
    for (i = 0; i < 2; i++) {
        printf("%d ", graph.vtxdist[i]);
    }

    printf("\nxadj\n");
    for (i = 0; i <= graph.vtxdist[1]; i++) {
        printf("%d ", graph.xadj[i]);
    }
    printf("\nadjcny\n");
    for (i = 0; i < graph.xadj[graph.vtxdist[1]]; i++) {
        printf("%d ", graph.adjncy[i]);
    }

    printf("\nwgtflag:%d\n", wgtflag);
    printf("numflag:%d\n", numflag);
    printf("ncon:%d\n", ncon);
    printf("nparts:%d\n", nparts);
    printf("tpwgts:\n");
    for (i = 0; i < nparts * ncon; i++) {
        printf("%f ", tpwgts[i]);
    }
    printf("\nubvec:\n");
    for (i = 0; i < ncon; i++) {
        printf("%f ", ubvec[i]);
    }
    printf("\noptions:\n");
    for (i = 0; i < 3; i++) {
        printf("%d ", options[i]);
    }
    printf("\n");



    char *path = (char*) malloc((strlen(prePath) + 20) * sizeof(char));
    sprintf(path, "%sG.%d", prePath, myid);
    FILE *fp = fopen(path, "r");

    int xx, yy;
    edge* e;
    e = (edge*)(malloc(1000 * sizeof(edge)));
    i = 0;
    while (~fscanf(fp, "%d%d", &xx, &yy)) {
        e[i].x = xx;
        e[i].y = yy;
        i++;
    }
    qsort(e, i, sizeof(edge), cmp);
    n = i;

    idx_t *vtxdist, *xadj, *adjcny;
    vtxdist = (idx_t*)malloc((numprocs + 3) * sizeof(idx_t));
    xadj = (idx_t*)malloc((vertexNum / numprocs + 3) * sizeof(idx_t));
    adjcny = (idx_t*)malloc((n + 3) * sizeof(idx_t));
    for (i = 0; i <= numprocs; i++) vtxdist[i] = vertexNum / numprocs * i;

    int bx = 0;
    i = 0;
    j = 0;
    int k = 0;
    xadj[j++] = 0;
    edge *it;
    for (k = 0; k < n; k++) {
        it = e + k;
        printf("x:%d y:%d\n", it->x, it->y);
        xx = it->x;
        yy = it->y;
        if (bx == xx) {
            if (i >0 && yy == adjcny[i - 1]) continue;
            adjcny[i++] = yy;
        } else {
            while (bx < xx) {
                bx++;
                xadj[j++] = i;
            }
            adjcny[i++] = yy;
        }
    }
    while (bx < vtxdist[myid + 1]) {
        bx++;
        xadj[j++] = i;
    }


    printf("\nxadj\n");
    for (i = 0; i <= vtxdist[1]; i++) {
        printf("%d ", xadj[i]);
    }
    printf("\nadjcny\n");
    for (i = 0; i < xadj[vtxdist[1]]; i++) {
        printf("%d ", adjcny[i]);
    }




    ParMETIS_V3_PartKway(vtxdist, xadj, adjcny, NULL,
                         NULL, &wgtflag, &numflag, &ncon, &nparts, tpwgts, ubvec, options,
                         &edgecut, part, &comm);

    printf("edgecut:%d\n", edgecut);
    printf("part:\n");
    for (i = 0; i < 16; i++) {
        printf("%d ", part[i]);
    }
    printf("\n");
    return;

}
