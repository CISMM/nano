#ifndef LINLEASTSQR_H
#define LINLEASTSQR_H

/* linearLeastSquaresSolve():
     This is a simplified interface to the CLAPACK function
     dgels() which frees you from using the f2c header file, 
     special FORTRAN data types and allocating work space for dgels.

     solves for the X that minimizes || B - A*X || and puts the result
     into the first n elements of B, the residual is the sum of the
     squares of the result values put in B[n+1] to B[m-1]
     (it is assumed that n <= m and that
     A is in the order col[0], col[1] ... col[n-1] where each col has m
     elements and B has m elements)
*/
int linearLeastSquaresSolve(int m, int n, double *A, double *B);

/* linearEqualityConstrainedLeastSquaresSolve():
     This is a simplified interface to the CLAPACK function
     dgglse() which frees you from using the f2c header file,
     special FORTRAN data types and allocating work space for dgglse

     solves for the X that minimizes || C - A*X ||_2 subject to the
     constraint B*X = D and puts the result into X
     A is an M by N matrix, B is a P by N matrix, C is an M-vector,
     D is a P-vector, where P <= N <= M+P

     matrices are in the order col[0], col[1] ... col[n-1] where each col 
     has m elements for A and p elements for B

     The error is returned in the elements C[n-p]..C[m-1]
*/

int linearEqualityConstrainedLeastSquaresSolve(int m, int n,
	int p, double *A, double *B, double *C, double *D, double *X);


#endif
