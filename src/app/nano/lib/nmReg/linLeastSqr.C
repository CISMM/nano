#include "f2c.h"

extern "C" int dgels_(char *trans, integer *m, integer *n, integer *
        nrhs, doublereal *a, integer *lda, doublereal *b, integer *ldb,
        doublereal *work, integer *lwork, integer *info);

int linearLeastSquaresSolve(int m, int n, double *A, double *B) {
    char trans = 'N'; /* says to minimize || B - A*X || */
    integer im = m;
    integer in = n;
    integer nrhs = 1;
    doublereal *drA;
    integer lda = m; /* = m? */
    doublereal *drB;
    integer ldb = m;
    doublereal *work;
    integer lwork = m*n;
    integer info;
    int result = 0;

    drA = new doublereal[m*n];
    drB = new doublereal[m];
    work = new doublereal[lwork];
    int i;
    for (i = 0; i < m*n; i++)
	drA[i] = A[i];
    for (i = 0; i < m; i++)
	drB[i] = B[i];

    dgels_(&trans, &im, &in, &nrhs, drA, &lda, drB, &ldb, work, &lwork, &info);

    for (i = 0; i < m*n; i++) 
	A[i] = drA[i];
    for (i = 0; i < m; i++)
	B[i] = drB[i];

    delete [] drA;
    delete [] drB;
    delete [] work;

    result = info;
    return result;
}

extern "C" int dgglse_(integer *m, integer *n, integer *p, doublereal *
        a, integer *lda, doublereal *b, integer *ldb, doublereal *c, 
        doublereal *d, doublereal *x, doublereal *work, integer *lwork, 
        integer *info);

int linearEqualityConstrainedLeastSquaresSolve(int m, int n, 
        int p, double *A, double *B, double *C, double *D, double *X) {

    integer im = m;
    integer in = n;
    integer ip = p;
    doublereal *drA;
    integer lda = m; /* = m? */
    doublereal *drB;
    integer ldb = p;
    doublereal *drC;
    doublereal *drD;
    doublereal *drX;
    doublereal *work;
    integer lwork = m+n+p;
    integer info;
    int result = 0;

    if (p == 0) ldb = 1;

    drA = new doublereal[m*n];
    drB = new doublereal[ldb*n];
    drC = new doublereal[m];
    drD = new doublereal[ldb];
    drX = new doublereal[n];
    work = new doublereal[lwork];
    int i;
    for (i = 0; i < m*n; i++)
        drA[i] = A[i];
    for (i = 0; i < p*n; i++)
        drB[i] = B[i];
    for (i = 0; i < m; i++)
	drC[i] = C[i];
    for (i = 0; i < p; i++)
	drD[i] = D[i];
    for (i = 0; i < n; i++)
	drX[i] = X[i];

    dgglse_(&im, &in, &ip, drA, &lda, drB, &ldb, drC, drD, drX, work, 
		&lwork, &info);

    for (i = 0; i < m*n; i++)
        A[i] = drA[i];
    for (i = 0; i < p*n; i++)
        B[i] = drB[i];
    for (i = 0; i < m; i++)
        C[i] = drC[i];
    for (i = 0; i < p; i++)
        D[i] = drD[i];
    for (i = 0; i < n; i++)
        X[i] = drX[i];


    delete [] drA;
    delete [] drB;
    delete [] drC;
    delete [] drD;
    delete [] drX;
    delete [] work;

    result = info;
    return result;
}
