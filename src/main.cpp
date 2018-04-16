#include "../include/parmetis.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>
#include <vector>
#include <iostream>
using namespace std;

const char* prePath = "/mnt/nfs/zpltys/tempDir/";
const int vertexNum = 32768;

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

int main (int argc, char *argv[]) {
    int myid, numprocs, namelen;
    int i, j;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);        /* starts MPI */
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);  /* get current process id */
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);      /* get number of processes */
    MPI_Get_processor_name(processor_name, &namelen);


    char *path = new char[strlen(prePath) + 20];
    sprintf(path, "%sG.%d", prePath, myid);
    FILE *fp = fopen(path, "r");

    idx_t x, y;
    vector<edge> e;
    while (~fscanf(fp, "%d%d", &x, &y)) {
        edge tmp;
        tmp.x = x;
        tmp.y = y;
        e.push_back(tmp);
    }
    sort(e.begin(), e.end(), cmp);

    delete[] path;

    idx_t *vtxdist, *xadj, *adjcny, wgtflag, numflag, ncon, nparts, *options, edgecut, *part;
    MPI_Comm comm;
    real_t *tpwgts, *ubvec;
    vtxdist = new idx_t[numprocs + 3];
    xadj = new idx_t[vertexNum / numprocs + 3];
    adjcny = new idx_t[e.size() + 3];
    for (i = 0; i <= numprocs; i++) vtxdist[i] = vertexNum / numprocs * i;

    idx_t bx = vtxdist[myid];
    i = 0;
    j = 0;
    xadj[j++] = 0;
    for (vector<edge>::iterator it = e.begin(); it != e.end(); it++) {
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
    nparts = 4;

    tpwgts = new real_t[1 * numprocs + 10];
    ubvec = new real_t[numprocs + 10];
    for (i = 0; i < 1 * numprocs; i++) {
        tpwgts[i] = 1.0 / numprocs;
        ubvec[i] = 1.05;
    }
    options = new idx_t[5];
    options[0] = 0;
    edgecut = 0;
    part = new idx_t[vertexNum / numprocs + 10];
    part[vertexNum / numprocs - 1] = 1;
    comm = MPI_COMM_WORLD;

    printf("process %d run func!\n", myid);
   /* if (myid == 1) {
        cout << "xadj: ";
        for (i = 0; i <= vertexNum / numprocs; i++) {
            cout << xadj[i] << " ";
        }
        cout << endl;
        cout << "adjcny: ";
        for (i = 0; i < vertexNum / numprocs; i++) {
            for (j = xadj[i]; j < xadj[i + 1]; j++) {
                cout << adjcny[j] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }*/
    cout << "vtxdist: ";
    for (i = 0; i <= numprocs; i++) {
        cout << vtxdist[i] << " ";
    }
    cout << endl;

    ParMETIS_V3_PartKway(vtxdist, xadj, adjcny, NULL, NULL, &wgtflag, &numflag, &ncon, &nparts, tpwgts, ubvec, options,
                         &edgecut, part, &comm);
    //for (i = 0; i <= numprocs; i++) vtxdist[i] = vertexNum / numprocs * i;
    for (i = 0; i <= vertexNum / numprocs; i++) {
        printf("part %d to %d\n", vtxdist[myid] + i, part[i]);
    }
    delete[] vtxdist;

    MPI_Finalize();
    return 0;
}
