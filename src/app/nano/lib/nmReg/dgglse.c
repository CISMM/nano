#ifdef __cplusplus
extern "C" {
#include <stdio.h>
#endif

#include "f2c.h"

/* Subroutine */ int dgglse_(integer *m, integer *n, integer *p, doublereal *
	a, integer *lda, doublereal *b, integer *ldb, doublereal *c, 
	doublereal *d, doublereal *x, doublereal *work, integer *lwork, 
	integer *info)
{
/*  -- LAPACK driver routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    DGGLSE solves the linear equality-constrained least squares (LSE)   
    problem:   

            minimize || c - A*x ||_2   subject to   B*x = d   

    where A is an M-by-N matrix, B is a P-by-N matrix, c is a given   
    M-vector, and d is a given P-vector. It is assumed that   
    P <= N <= M+P, and   

             rank(B) = P and  rank( ( A ) ) = N.   
                                  ( ( B ) )   

    These conditions ensure that the LSE problem has a unique solution,   
    which is obtained using a GRQ factorization of the matrices B and A. 
  

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrices A and B. N >= 0.   

    P       (input) INTEGER   
            The number of rows of the matrix B. 0 <= P <= N <= M+P.   

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)   
            On entry, the M-by-N matrix A.   
            On exit, A is destroyed.   

    LDA     (input) INTEGER   
            The leading dimension of the array A. LDA >= max(1,M).   

    B       (input/output) DOUBLE PRECISION array, dimension (LDB,N)   
            On entry, the P-by-N matrix B.   
            On exit, B is destroyed.   

    LDB     (input) INTEGER   
            The leading dimension of the array B. LDB >= max(1,P).   

    C       (input/output) DOUBLE PRECISION array, dimension (M)   
            On entry, C contains the right hand side vector for the   
            least squares part of the LSE problem.   
            On exit, the residual sum of squares for the solution   
            is given by the sum of squares of elements N-P+1 to M of   
            vector C.   

    D       (input/output) DOUBLE PRECISION array, dimension (P)   
            On entry, D contains the right hand side vector for the   
            constrained equation.   
            On exit, D is destroyed.   

    X       (output) DOUBLE PRECISION array, dimension (N)   
            On exit, X is the solution of the LSE problem.   

    WORK    (workspace/output) DOUBLE PRECISION array, dimension (LWORK) 
  
            On exit, if INFO = 0, WORK(1) returns the optimal LWORK.   

    LWORK   (input) INTEGER   
            The dimension of the array WORK. LWORK >= max(1,M+N+P).   
            For optimum performance LWORK >= P+min(M,N)+max(M,N)*NB,   
            where NB is an upper bound for the optimal blocksizes for   
            DGEQRF, SGERQF, DORMQR and SORMRQ.   

    INFO    (output) INTEGER   
            = 0:  successful exit.   
            < 0:  if INFO = -i, the i-th argument had an illegal value.   

    ===================================================================== 
  


       Test the input parameters   

    
   Parameter adjustments   
       Function Body */
    /* Table of constant values */
    static integer c__1 = 1;
    static doublereal c_b11 = -1.;
    static doublereal c_b13 = 1.;
    
    /* System generated locals */
    //integer a_dim1, a_offset, b_dim1, b_offset, 
	integer i__1, i__2;
    /* Local variables */
    static integer lopt;
    extern /* Subroutine */ int dgemv_(char *, integer *, integer *, 
	    doublereal *, doublereal *, integer *, doublereal *, integer *, 
	    doublereal *, doublereal *, integer *), dcopy_(integer *, 
	    doublereal *, integer *, doublereal *, integer *), daxpy_(integer 
	    *, doublereal *, doublereal *, integer *, doublereal *, integer *)
	    , dtrmv_(char *, char *, char *, integer *, doublereal *, integer 
	    *, doublereal *, integer *), dtrsv_(char *
	    , char *, char *, integer *, doublereal *, integer *, doublereal *
	    , integer *);
    static integer mn, nr;
    extern /* Subroutine */ int dggrqf_(integer *, integer *, integer *, 
	    doublereal *, integer *, doublereal *, doublereal *, integer *, 
	    doublereal *, doublereal *, integer *, integer *), xerbla_(char *,
	     integer *), dormqr_(char *, char *, integer *, integer *,
	     integer *, doublereal *, integer *, doublereal *, doublereal *, 
	    integer *, doublereal *, integer *, integer *), 
	    dormrq_(char *, char *, integer *, integer *, integer *, 
	    doublereal *, integer *, doublereal *, doublereal *, integer *, 
	    doublereal *, integer *, integer *);



#define C(I) c[(I)-1]
#define D(I) d[(I)-1]
#define X(I) x[(I)-1]
#define WORK(I) work[(I)-1]

#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]
#define B(I,J) b[(I)-1 + ((J)-1)* ( *ldb)]

    *info = 0;
    mn = min(*m,*n);
    if (*m < 0) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*p < 0 || *p > *n || *p < *n - *m) {
	*info = -3;
    } else if (*lda < max(1,*m)) {
	*info = -5;
    } else if (*ldb < max(1,*p)) {
	*info = -7;
    } else /* if(complicated condition) */ {
/* Computing MAX */
	i__1 = 1, i__2 = *m + *n + *p;
	if (*lwork < max(i__1,i__2)) {
	    *info = -12;
	}
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DGGLSE", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }

/*     Compute the GRQ factorization of matrices B and A:   

              B*Q' = (  0  T12 ) P   Z'*A*Q' = ( R11 R12 ) N-P   
                       N-P  P                  (  0  R22 ) M+P-N   
                                                 N-P  P   

       where T12 and R11 are upper triangular, and Q and Z are   
       orthogonal. */

    i__1 = *lwork - *p - mn;
    dggrqf_(p, m, n, &B(1,1), ldb, &WORK(1), &A(1,1), lda, &WORK(*p 
	    + 1), &WORK(*p + mn + 1), &i__1, info);
    lopt = (integer) WORK(*p + mn + 1);

/*     Update c = Z'*c = ( c1 ) N-P   
                         ( c2 ) M+P-N */

    i__1 = max(1,*m);
    i__2 = *lwork - *p - mn;
    dormqr_("Left", "Transpose", m, &c__1, &mn, &A(1,1), lda, &WORK(*p + 
	    1), &C(1), &i__1, &WORK(*p + mn + 1), &i__2, info);
/* Computing MAX */
    i__1 = lopt, i__2 = (integer) WORK(*p + mn + 1);
    lopt = max(i__1,i__2);

/*     Solve T12*x2 = d for x2 */

    dtrsv_("Upper", "No transpose", "Non unit", p, &B(1,*n-*p+1), ldb, &D(1), &c__1);

/*     Update c1 */

    i__1 = *n - *p;
    dgemv_("No transpose", &i__1, p, &c_b11, &A(1,*n-*p+1), 
	    lda, &D(1), &c__1, &c_b13, &C(1), &c__1);

/*     Sovle R11*x1 = c1 for x1 */

    i__1 = *n - *p;
    dtrsv_("Upper", "No transpose", "Non unit", &i__1, &A(1,1), lda, &C(
	    1), &c__1);

/*     Put the solutions in X */

    i__1 = *n - *p;
    dcopy_(&i__1, &C(1), &c__1, &X(1), &c__1);
    dcopy_(p, &D(1), &c__1, &X(*n - *p + 1), &c__1);

/*     Compute the residual vector: */

    if (*m < *n) {
	nr = *m + *p - *n;
	i__1 = *n - *m;
	dgemv_("No transpose", &nr, &i__1, &c_b11, &A(*n-*p+1,*m+1), lda, &D(nr + 1), &c__1, &c_b13, &C(*n - *p + 1), &
		c__1);
    } else {
	nr = *p;
    }
    dtrmv_("Upper", "No transpose", "Non unit", &nr, &A(*n-*p+1,*n-*p+1), lda, &D(1), &c__1);
    daxpy_(&nr, &c_b11, &D(1), &c__1, &C(*n - *p + 1), &c__1);

/*     Backward transformation x = Q'*x */

    i__1 = *lwork - *p - mn;
    dormrq_("Left", "Transpose", n, &c__1, p, &B(1,1), ldb, &WORK(1), &X(
	    1), n, &WORK(*p + mn + 1), &i__1, info);
/* Computing MAX */
    i__1 = lopt, i__2 = (integer) WORK(*p + mn + 1);
    WORK(1) = (doublereal) (*p + mn + max(i__1,i__2));

    return 0;

/*     End of DGGLSE */

} /* dgglse_ */

#include "f2c.h"

/* Subroutine */ int dgerq2_(integer *m, integer *n, doublereal *a, integer *
	lda, doublereal *tau, doublereal *work, integer *info)
{
/*  -- LAPACK routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    DGERQ2 computes an RQ factorization of a real m by n matrix A:   
    A = R * Q.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix A.  N >= 0.   

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)   
            On entry, the m by n matrix A.   
            On exit, if m <= n, the upper triangle of the subarray   
            A(1:m,n-m+1:n) contains the m by m upper triangular matrix R; 
  
            if m >= n, the elements on and above the (m-n)-th subdiagonal 
  
            contain the m by n upper trapezoidal matrix R; the remaining 
  
            elements, with the array TAU, represent the orthogonal matrix 
  
            Q as a product of elementary reflectors (see Further   
            Details).   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(1,M).   

    TAU     (output) DOUBLE PRECISION array, dimension (min(M,N))   
            The scalar factors of the elementary reflectors (see Further 
  
            Details).   

    WORK    (workspace) DOUBLE PRECISION array, dimension (M)   

    INFO    (output) INTEGER   
            = 0: successful exit   
            < 0: if INFO = -i, the i-th argument had an illegal value   

    Further Details   
    ===============   

    The matrix Q is represented as a product of elementary reflectors   

       Q = H(1) H(2) . . . H(k), where k = min(m,n).   

    Each H(i) has the form   

       H(i) = I - tau * v * v'   

    where tau is a real scalar, and v is a real vector with   
    v(n-k+i+1:n) = 0 and v(n-k+i) = 1; v(1:n-k+i-1) is stored on exit in 
  
    A(m-k+i,1:n-k+i-1), and tau in TAU(i).   

    ===================================================================== 
  


       Test the input arguments   

    
   Parameter adjustments   
       Function Body */
    /* System generated locals */
    //integer a_dim1, a_offset, 
	integer i__1, i__2;
    /* Local variables */
    static integer i, k;
    extern /* Subroutine */ int dlarf_(char *, integer *, integer *, 
	    doublereal *, integer *, doublereal *, doublereal *, integer *, 
	    doublereal *), dlarfg_(integer *, doublereal *, 
	    doublereal *, integer *, doublereal *), xerbla_(char *, integer *);
    static doublereal aii;


#define TAU(I) tau[(I)-1]
#define WORK(I) work[(I)-1]

#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]

    *info = 0;
    if (*m < 0) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*lda < max(1,*m)) {
	*info = -4;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DGERQ2", &i__1);
	return 0;
    }

    k = min(*m,*n);

    for (i = k; i >= 1; --i) {

/*        Generate elementary reflector H(i) to annihilate   
          A(m-k+i,1:n-k+i-1) */

	i__1 = *n - k + i;
	dlarfg_(&i__1, &A(*m-k+i,*n-k+i), &A(*m-k+i,1), lda, &TAU(i));

/*        Apply H(i) to A(1:m-k+i-1,1:n-k+i) from the right */

	aii = A(*m-k+i,*n-k+i);
	A(*m-k+i,*n-k+i) = 1.;
	i__1 = *m - k + i - 1;
	i__2 = *n - k + i;
	dlarf_("Right", &i__1, &i__2, &A(*m-k+i,1), lda, &TAU(i), &
		A(1,1), lda, &WORK(1));
	A(*m-k+i,*n-k+i) = aii;
/* L10: */
    }
    return 0;

/*     End of DGERQ2 */

} /* dgerq2_ */

#include "f2c.h"

/* Subroutine */ int dgerqf_(integer *m, integer *n, doublereal *a, integer *
	lda, doublereal *tau, doublereal *work, integer *lwork, integer *info)
{
/*  -- LAPACK routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    DGERQF computes an RQ factorization of a real M-by-N matrix A:   
    A = R * Q.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix A.  N >= 0.   

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)   
            On entry, the M-by-N matrix A.   
            On exit,   
            if m <= n, the upper triangle of the subarray   
            A(1:m,n-m+1:n) contains the M-by-M upper triangular matrix R; 
  
            if m >= n, the elements on and above the (m-n)-th subdiagonal 
  
            contain the M-by-N upper trapezoidal matrix R;   
            the remaining elements, with the array TAU, represent the   
            orthogonal matrix Q as a product of min(m,n) elementary   
            reflectors (see Further Details).   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(1,M).   

    TAU     (output) DOUBLE PRECISION array, dimension (min(M,N))   
            The scalar factors of the elementary reflectors (see Further 
  
            Details).   

    WORK    (workspace/output) DOUBLE PRECISION array, dimension (LWORK) 
  
            On exit, if INFO = 0, WORK(1) returns the optimal LWORK.   

    LWORK   (input) INTEGER   
            The dimension of the array WORK.  LWORK >= max(1,M).   
            For optimum performance LWORK >= M*NB, where NB is   
            the optimal blocksize.   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   

    Further Details   
    ===============   

    The matrix Q is represented as a product of elementary reflectors   

       Q = H(1) H(2) . . . H(k), where k = min(m,n).   

    Each H(i) has the form   

       H(i) = I - tau * v * v'   

    where tau is a real scalar, and v is a real vector with   
    v(n-k+i+1:n) = 0 and v(n-k+i) = 1; v(1:n-k+i-1) is stored on exit in 
  
    A(m-k+i,1:n-k+i-1), and tau in TAU(i).   

    ===================================================================== 
  


       Test the input arguments   

    
   Parameter adjustments   
       Function Body */
    /* Table of constant values */
    static integer c__1 = 1;
    static integer c_n1 = -1;
    static integer c__3 = 3;
    static integer c__2 = 2;
    
    /* System generated locals */
    //integer a_dim1, a_offset, 
	integer i__1, i__2, i__3, i__4;
    /* Local variables */
    static integer i, k, nbmin, iinfo;
    extern /* Subroutine */ int dgerq2_(integer *, integer *, doublereal *, 
	    integer *, doublereal *, doublereal *, integer *);
    static integer ib, nb, ki, kk;
    extern /* Subroutine */ int dlarfb_(char *, char *, char *, char *, 
	    integer *, integer *, integer *, doublereal *, integer *, 
	    doublereal *, integer *, doublereal *, integer *, doublereal *, 
	    integer *);
    static integer mu, nu, nx;
    extern /* Subroutine */ int dlarft_(char *, char *, integer *, integer *, 
	    doublereal *, integer *, doublereal *, doublereal *, integer *), xerbla_(char *, integer *);
    extern integer ilaenv_(integer *, char *, char *, integer *, integer *, 
	    integer *, integer *, ftnlen, ftnlen);
    static integer ldwork, iws;



#define TAU(I) tau[(I)-1]
#define WORK(I) work[(I)-1]

#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]

    *info = 0;
    if (*m < 0) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*lda < max(1,*m)) {
	*info = -4;
    } else if (*lwork < max(1,*m)) {
	*info = -7;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DGERQF", &i__1);
	return 0;
    }

/*     Quick return if possible */

    k = min(*m,*n);
    if (k == 0) {
	WORK(1) = 1.;
	return 0;
    }

/*     Determine the block size. */

    nb = ilaenv_(&c__1, "DGERQF", " ", m, n, &c_n1, &c_n1, 6L, 1L);
    nbmin = 2;
    nx = 1;
    iws = *m;
    if (nb > 1 && nb < k) {

/*        Determine when to cross over from blocked to unblocked code.
   

   Computing MAX */
	i__1 = 0, i__2 = ilaenv_(&c__3, "DGERQF", " ", m, n, &c_n1, &c_n1, 6L,
		 1L);
	nx = max(i__1,i__2);
	if (nx < k) {

/*           Determine if workspace is large enough for blocked co
de. */

	    ldwork = *m;
	    iws = ldwork * nb;
	    if (*lwork < iws) {

/*              Not enough workspace to use optimal NB:  reduc
e NB and   
                determine the minimum value of NB. */

		nb = *lwork / ldwork;
/* Computing MAX */
		i__1 = 2, i__2 = ilaenv_(&c__2, "DGERQF", " ", m, n, &c_n1, &
			c_n1, 6L, 1L);
		nbmin = max(i__1,i__2);
	    }
	}
    }

    if (nb >= nbmin && nb < k && nx < k) {

/*        Use blocked code initially.   
          The last kk rows are handled by the block method. */

	ki = (k - nx - 1) / nb * nb;
/* Computing MIN */
	i__1 = k, i__2 = ki + nb;
	kk = min(i__1,i__2);

	i__1 = k - kk + 1;
	i__2 = -nb;
	for (i = k - kk + ki + 1; -nb < 0 ? i >= k-kk+1 : i <= k-kk+1; i += -nb)
		 {
/* Computing MIN */
	    i__3 = k - i + 1;
	    ib = min(i__3,nb);

/*           Compute the RQ factorization of the current block   
             A(m-k+i:m-k+i+ib-1,1:n-k+i+ib-1) */

	    i__3 = *n - k + i + ib - 1;
	    dgerq2_(&ib, &i__3, &A(*m-k+i,1), lda, &TAU(i), &WORK(
		    1), &iinfo);
	    if (*m - k + i > 1) {

/*              Form the triangular factor of the block reflec
tor   
                H = H(i+ib-1) . . . H(i+1) H(i) */

		i__3 = *n - k + i + ib - 1;
		dlarft_("Backward", "Rowwise", &i__3, &ib, &A(*m-k+i,1), lda, &TAU(i), &WORK(1), &ldwork);

/*              Apply H to A(1:m-k+i-1,1:n-k+i+ib-1) from the 
right */

		i__3 = *m - k + i - 1;
		i__4 = *n - k + i + ib - 1;
		dlarfb_("Right", "No transpose", "Backward", "Rowwise", &i__3,
			 &i__4, &ib, &A(*m-k+i,1), lda, &WORK(1), &
			ldwork, &A(1,1), lda, &WORK(ib + 1), &ldwork);
	    }
/* L10: */
	}
	mu = *m - k + i + nb - 1;
	nu = *n - k + i + nb - 1;
    } else {
	mu = *m;
	nu = *n;
    }

/*     Use unblocked code to factor the last or only block */

    if (mu > 0 && nu > 0) {
	dgerq2_(&mu, &nu, &A(1,1), lda, &TAU(1), &WORK(1), &iinfo);
    }

    WORK(1) = (doublereal) iws;
    return 0;

/*     End of DGERQF */

} /* dgerqf_ */

#include "f2c.h"

/* Subroutine */ int dggrqf_(integer *m, integer *p, integer *n, doublereal *
	a, integer *lda, doublereal *taua, doublereal *b, integer *ldb, 
	doublereal *taub, doublereal *work, integer *lwork, integer *info)
{
/*  -- LAPACK routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    DGGRQF computes a generalized RQ factorization of an M-by-N matrix A 
  
    and a P-by-N matrix B:   

                A = R*Q,        B = Z*T*Q,   

    where Q is an N-by-N orthogonal matrix, Z is a P-by-P orthogonal   
    matrix, and R and T assume one of the forms:   

    if M <= N,  R = ( 0  R12 ) M,   or if M > N,  R = ( R11 ) M-N,   
                     N-M  M                           ( R21 ) N   
                                                         N   

    where R12 or R21 is upper triangular, and   

    if P >= N,  T = ( T11 ) N  ,   or if P < N,  T = ( T11  T12 ) P,   
                    (  0  ) P-N                         P   N-P   
                       N   

    where T11 is upper triangular.   

    In particular, if B is square and nonsingular, the GRQ factorization 
  
    of A and B implicitly gives the RQ factorization of A*inv(B):   

                 A*inv(B) = (R*inv(T))*Z'   

    where inv(B) denotes the inverse of the matrix B, and Z' denotes the 
  
    transpose of the matrix Z.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    P       (input) INTEGER   
            The number of rows of the matrix B.  P >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrices A and B. N >= 0.   

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)   
            On entry, the M-by-N matrix A.   
            On exit, if M <= N, the upper triangle of the subarray   
            A(1:M,N-M+1:N) contains the M-by-M upper triangular matrix R; 
  
            if M > N, the elements on and above the (M-N)-th subdiagonal 
  
            contain the M-by-N upper trapezoidal matrix R; the remaining 
  
            elements, with the array TAUA, represent the orthogonal   
            matrix Q as a product of elementary reflectors (see Further   
            Details).   

    LDA     (input) INTEGER   
            The leading dimension of the array A. LDA >= max(1,M).   

    TAUA    (output) DOUBLE PRECISION array, dimension (min(M,N))   
            The scalar factors of the elementary reflectors which   
            represent the orthogonal matrix Q (see Further Details).   

    B       (input/output) DOUBLE PRECISION array, dimension (LDB,N)   
            On entry, the P-by-N matrix B.   
            On exit, the elements on and above the diagonal of the array 
  
            contain the min(P,N)-by-N upper trapezoidal matrix T (T is   
            upper triangular if P >= N); the elements below the diagonal, 
  
            with the array TAUB, represent the orthogonal matrix Z as a   
            product of elementary reflectors (see Further Details).   

    LDB     (input) INTEGER   
            The leading dimension of the array B. LDB >= max(1,P).   

    TAUB    (output) DOUBLE PRECISION array, dimension (min(P,N))   
            The scalar factors of the elementary reflectors which   
            represent the orthogonal matrix Z (see Further Details).   

    WORK    (workspace/output) DOUBLE PRECISION array, dimension (LWORK) 
  
            On exit, if INFO = 0, WORK(1) returns the optimal LWORK.   

    LWORK   (input) INTEGER   
            The dimension of the array WORK. LWORK >= max(1,N,M,P).   
            For optimum performance LWORK >= max(N,M,P)*max(NB1,NB2,NB3), 
  
            where NB1 is the optimal blocksize for the RQ factorization   
            of an M-by-N matrix, NB2 is the optimal blocksize for the   
            QR factorization of a P-by-N matrix, and NB3 is the optimal   
            blocksize for a call of DORMRQ.   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INF0= -i, the i-th argument had an illegal value.   

    Further Details   
    ===============   

    The matrix Q is represented as a product of elementary reflectors   

       Q = H(1) H(2) . . . H(k), where k = min(m,n).   

    Each H(i) has the form   

       H(i) = I - taua * v * v'   

    where taua is a real scalar, and v is a real vector with   
    v(n-k+i+1:n) = 0 and v(n-k+i) = 1; v(1:n-k+i-1) is stored on exit in 
  
    A(m-k+i,1:n-k+i-1), and taua in TAUA(i).   
    To form Q explicitly, use LAPACK subroutine DORGRQ.   
    To use Q to update another matrix, use LAPACK subroutine DORMRQ.   

    The matrix Z is represented as a product of elementary reflectors   

       Z = H(1) H(2) . . . H(k), where k = min(p,n).   

    Each H(i) has the form   

       H(i) = I - taub * v * v'   

    where taub is a real scalar, and v is a real vector with   
    v(1:i-1) = 0 and v(i) = 1; v(i+1:p) is stored on exit in B(i+1:p,i), 
  
    and taub in TAUB(i).   
    To form Z explicitly, use LAPACK subroutine DORGQR.   
    To use Z to update another matrix, use LAPACK subroutine DORMQR.   

    ===================================================================== 
  


       Test the input parameters   

    
   Parameter adjustments   
       Function Body */
    /* System generated locals */
    //integer a_dim1, a_offset, b_dim1, b_offset, 
	integer i__1, i__2;//, i__3;
    /* Local variables */
    static integer lopt;
    extern /* Subroutine */ int dgeqrf_(integer *, integer *, doublereal *, 
	    integer *, doublereal *, doublereal *, integer *, integer *), 
	    dgerqf_(integer *, integer *, doublereal *, integer *, doublereal 
	    *, doublereal *, integer *, integer *), xerbla_(char *, integer *), dormrq_(char *, char *, integer *, integer *, integer *,
	     doublereal *, integer *, doublereal *, doublereal *, integer *, 
	    doublereal *, integer *, integer *);


#define TAUA(I) taua[(I)-1]
#define TAUB(I) taub[(I)-1]
#define WORK(I) work[(I)-1]

#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]
#define B(I,J) b[(I)-1 + ((J)-1)* ( *ldb)]

    *info = 0;
    if (*m < 0) {
	*info = -1;
    } else if (*p < 0) {
	*info = -2;
    } else if (*n < 0) {
	*info = -3;
    } else if (*lda < max(1,*m)) {
	*info = -5;
    } else if (*ldb < max(1,*p)) {
	*info = -8;
    } else /* if(complicated condition) */ {
/* Computing MAX */
	i__1 = max(1,*m), i__1 = max(i__1,*p);
	if (*lwork < max(i__1,*n)) {
	    *info = -11;
	}
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DGGRQF", &i__1);
	return 0;
    }

/*     RQ factorization of M-by-N matrix A: A = R*Q */

    dgerqf_(m, n, &A(1,1), lda, &TAUA(1), &WORK(1), lwork, info);
    lopt = (integer) WORK(1);

/*     Update B := B*Q' */

    i__1 = min(*m,*n);
/* Computing MAX */
    i__2 = 1, //i__3 = *m - *n + 1;
    dormrq_("Right", "Transpose", p, n, &i__1, &A(max(1,*m-*n+1),1), 
	    lda, &TAUA(1), &B(1,1), ldb, &WORK(1), lwork, info);
/* Computing MAX */
    i__1 = lopt, i__2 = (integer) WORK(1);
    lopt = max(i__1,i__2);

/*     QR factorization of P-by-N matrix B: B = Z*T */

    dgeqrf_(p, n, &B(1,1), ldb, &TAUB(1), &WORK(1), lwork, info);
/* Computing MAX */
    i__1 = lopt, i__2 = (integer) WORK(1);
    WORK(1) = (doublereal) max(i__1,i__2);

    return 0;

/*     End of DGGRQF */

} /* dggrqf_ */

#include "f2c.h"

/* Subroutine */ int dormr2_(char *side, char *trans, integer *m, integer *n, 
	integer *k, doublereal *a, integer *lda, doublereal *tau, doublereal *
	c, integer *ldc, doublereal *work, integer *info)
{
/*  -- LAPACK routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    DORMR2 overwrites the general real m by n matrix C with   

          Q * C  if SIDE = 'L' and TRANS = 'N', or   

          Q'* C  if SIDE = 'L' and TRANS = 'T', or   

          C * Q  if SIDE = 'R' and TRANS = 'N', or   

          C * Q' if SIDE = 'R' and TRANS = 'T',   

    where Q is a real orthogonal matrix defined as the product of k   
    elementary reflectors   

          Q = H(1) H(2) . . . H(k)   

    as returned by DGERQF. Q is of order m if SIDE = 'L' and of order n   
    if SIDE = 'R'.   

    Arguments   
    =========   

    SIDE    (input) CHARACTER*1   
            = 'L': apply Q or Q' from the Left   
            = 'R': apply Q or Q' from the Right   

    TRANS   (input) CHARACTER*1   
            = 'N': apply Q  (No transpose)   
            = 'T': apply Q' (Transpose)   

    M       (input) INTEGER   
            The number of rows of the matrix C. M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix C. N >= 0.   

    K       (input) INTEGER   
            The number of elementary reflectors whose product defines   
            the matrix Q.   
            If SIDE = 'L', M >= K >= 0;   
            if SIDE = 'R', N >= K >= 0.   

    A       (input) DOUBLE PRECISION array, dimension   
                                 (LDA,M) if SIDE = 'L',   
                                 (LDA,N) if SIDE = 'R'   
            The i-th row must contain the vector which defines the   
            elementary reflector H(i), for i = 1,2,...,k, as returned by 
  
            DGERQF in the last k rows of its array argument A.   
            A is modified by the routine but restored on exit.   

    LDA     (input) INTEGER   
            The leading dimension of the array A. LDA >= max(1,K).   

    TAU     (input) DOUBLE PRECISION array, dimension (K)   
            TAU(i) must contain the scalar factor of the elementary   
            reflector H(i), as returned by DGERQF.   

    C       (input/output) DOUBLE PRECISION array, dimension (LDC,N)   
            On entry, the m by n matrix C.   
            On exit, C is overwritten by Q*C or Q'*C or C*Q' or C*Q.   

    LDC     (input) INTEGER   
            The leading dimension of the array C. LDC >= max(1,M).   

    WORK    (workspace) DOUBLE PRECISION array, dimension   
                                     (N) if SIDE = 'L',   
                                     (M) if SIDE = 'R'   

    INFO    (output) INTEGER   
            = 0: successful exit   
            < 0: if INFO = -i, the i-th argument had an illegal value   

    ===================================================================== 
  


       Test the input arguments   

    
   Parameter adjustments   
       Function Body */
    /* System generated locals */
    //integer a_dim1, a_offset, c_dim1, c_offset, 
	integer i__1;//, i__2;
    /* Local variables */
    static logical left;
    static integer i;
    extern /* Subroutine */ int dlarf_(char *, integer *, integer *, 
	    doublereal *, integer *, doublereal *, doublereal *, integer *, 
	    doublereal *);
    extern logical lsame_(char *, char *);
    static integer i1, i2, i3, mi, ni, nq;
    extern /* Subroutine */ int xerbla_(char *, integer *);
    static logical notran;
    static doublereal aii;

#undef C

#define TAU(I) tau[(I)-1]
#define WORK(I) work[(I)-1]

#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]
#define C(I,J) c[(I)-1 + ((J)-1)* ( *ldc)]

    *info = 0;
    left = lsame_(side, "L");
    notran = lsame_(trans, "N");

/*     NQ is the order of Q */

    if (left) {
	nq = *m;
    } else {
	nq = *n;
    }
    if (! left && ! lsame_(side, "R")) {
	*info = -1;
    } else if (! notran && ! lsame_(trans, "T")) {
	*info = -2;
    } else if (*m < 0) {
	*info = -3;
    } else if (*n < 0) {
	*info = -4;
    } else if (*k < 0 || *k > nq) {
	*info = -5;
    } else if (*lda < max(1,*k)) {
	*info = -7;
    } else if (*ldc < max(1,*m)) {
	*info = -10;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DORMR2", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*m == 0 || *n == 0 || *k == 0) {
	return 0;
    }

    if (left && ! notran || ! left && notran) {
	i1 = 1;
	i2 = *k;
	i3 = 1;
    } else {
	i1 = *k;
	i2 = 1;
	i3 = -1;
    }

    if (left) {
	ni = *n;
    } else {
	mi = *m;
    }

    i__1 = i2;
    //i__2 = i3;
    for (i = i1; i3 < 0 ? i >= i2 : i <= i2; i += i3) {
	if (left) {

/*           H(i) is applied to C(1:m-k+i,1:n) */

	    mi = *m - *k + i;
	} else {

/*           H(i) is applied to C(1:m,1:n-k+i) */

	    ni = *n - *k + i;
	}

/*        Apply H(i) */

	aii = A(i,nq-*k+i);
	A(i,nq-*k+i) = 1.;
	dlarf_(side, &mi, &ni, &A(i,1), lda, &TAU(i), &C(1,1), 
		ldc, &WORK(1));
	A(i,nq-*k+i) = aii;
/* L10: */
    }
    return 0;

/*     End of DORMR2 */

} /* dormr2_ */


/* Subroutine */ int dormrq_(char *side, char *trans, integer *m, integer *n,
        integer *k, doublereal *a, integer *lda, doublereal *tau, doublereal *
        c, integer *ldc, doublereal *work, integer *lwork, integer *info)
{
/*  -- LAPACK routine (version 2.0) --
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
       Courant Institute, Argonne National Lab, and Rice University
       September 30, 1994


    Purpose
    =======

    DORMRQ overwrites the general real M-by-N matrix C with

                    SIDE = 'L'     SIDE = 'R'
    TRANS = 'N':      Q * C          C * Q
    TRANS = 'T':      Q**T * C       C * Q**T

    where Q is a real orthogonal matrix defined as the product of k
    elementary reflectors

          Q = H(1) H(2) . . . H(k)

    as returned by DGERQF. Q is of order M if SIDE = 'L' and of order N
    if SIDE = 'R'.
    Arguments
    =========

    SIDE    (input) CHARACTER*1
            = 'L': apply Q or Q**T from the Left;
            = 'R': apply Q or Q**T from the Right.

    TRANS   (input) CHARACTER*1
            = 'N':  No transpose, apply Q;
            = 'T':  Transpose, apply Q**T.

    M       (input) INTEGER
            The number of rows of the matrix C. M >= 0.

    N       (input) INTEGER
            The number of columns of the matrix C. N >= 0.

    K       (input) INTEGER
            The number of elementary reflectors whose product defines
            the matrix Q.
            If SIDE = 'L', M >= K >= 0;
            if SIDE = 'R', N >= K >= 0.

    A       (input) DOUBLE PRECISION array, dimension
                                 (LDA,M) if SIDE = 'L',
                                 (LDA,N) if SIDE = 'R'
            The i-th row must contain the vector which defines the
            elementary reflector H(i), for i = 1,2,...,k, as returned by
 
            DGERQF in the last k rows of its array argument A.
            A is modified by the routine but restored on exit.

    LDA     (input) INTEGER
            The leading dimension of the array A. LDA >= max(1,K).

    TAU     (input) DOUBLE PRECISION array, dimension (K)
            TAU(i) must contain the scalar factor of the elementary
            reflector H(i), as returned by DGERQF.

    C       (input/output) DOUBLE PRECISION array, dimension (LDC,N)
            On entry, the M-by-N matrix C.
            On exit, C is overwritten by Q*C or Q**T*C or C*Q**T or C*Q.
 

    LDC     (input) INTEGER
            The leading dimension of the array C. LDC >= max(1,M).

    WORK    (workspace/output) DOUBLE PRECISION array, dimension (LWORK)
 
            On exit, if INFO = 0, WORK(1) returns the optimal LWORK.

    LWORK   (input) INTEGER
            The dimension of the array WORK.
            If SIDE = 'L', LWORK >= max(1,N);
            if SIDE = 'R', LWORK >= max(1,M).
            For optimum performance LWORK >= N*NB if SIDE = 'L', and
            LWORK >= M*NB if SIDE = 'R', where NB is the optimal
            blocksize.

    INFO    (output) INTEGER
            = 0:  successful exit
            < 0:  if INFO = -i, the i-th argument had an illegal value

    =====================================================================
 


       Test the input arguments

   
   Parameter adjustments
       Function Body */
    /* Table of constant values */
    static integer c__1 = 1;
    static integer c_n1 = -1;
    static integer c__2 = 2;
    static integer c__65 = 65;

    /* System generated locals */
    address a__1[2];
    //integer a_dim1, a_offset, c_dim1, c_offset, 
	integer i__1, i__2, i__3[2], i__4, i__5;
    char ch__1[2];
    /* Builtin functions
       Subroutine */ int s_cat(char *, char **, integer *, integer *, ftnlen);
    /* Local variables */
    static logical left;
    static integer i;
    static doublereal t[4160]   /* was [65][64] */;
    extern logical lsame_(char *, char *);
    static integer nbmin, iinfo, i1, i2, i3;
    extern /* Subroutine */ int dormr2_(char *, char *, integer *, integer *,
            integer *, doublereal *, integer *, doublereal *, doublereal *,
            integer *, doublereal *, integer *);
    static integer ib, nb, mi, ni;
    extern /* Subroutine */ int dlarfb_(char *, char *, char *, char *,
            integer *, integer *, integer *, doublereal *, integer *,
            doublereal *, integer *, doublereal *, integer *, doublereal *,
            integer *);
    static integer nq, nw;
    extern /* Subroutine */ int dlarft_(char *, char *, integer *, integer *,
            doublereal *, integer *, doublereal *, doublereal *, integer *), 
	xerbla_(char *, integer *);
    extern integer ilaenv_(integer *, char *, char *, integer *, integer *,
            integer *, integer *, ftnlen, ftnlen);
    static logical notran;
    static integer ldwork;
    static char transt[1];
    static integer iws;



#define T(I) t[(I)]
#define WAS(I) was[(I)]
#define TAU(I) tau[(I)-1]
#define WORK(I) work[(I)-1]

#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]
#define C(I,J) c[(I)-1 + ((J)-1)* ( *ldc)]

    *info = 0;
    left = lsame_(side, "L");
    notran = lsame_(trans, "N");

/*     NQ is the order of Q and NW is the minimum dimension of WORK */

    if (left) {
        nq = *m;
        nw = *n;
    } else {
        nq = *n;
        nw = *m;
    }
    if (! left && ! lsame_(side, "R")) {
        *info = -1;
    } else if (! notran && ! lsame_(trans, "T")) {
        *info = -2;
    } else if (*m < 0) {
        *info = -3;
    } else if (*n < 0) {
        *info = -4;
    } else if (*k < 0 || *k > nq) {
        *info = -5;
    } else if (*lda < max(1,*k)) {
        *info = -7;
    } else if (*ldc < max(1,*m)) {
        *info = -10;
    } else if (*lwork < max(1,nw)) {
        *info = -12;
    }
    if (*info != 0) {
        i__1 = -(*info);
        xerbla_("DORMRQ", &i__1);
        return 0;
    }

/*     Quick return if possible */

    if (*m == 0 || *n == 0 || *k == 0) {
        WORK(1) = 1.;
        return 0;
    }

/*     Determine the block size.  NB may be at most NBMAX, where NBMAX
       is used to define the local array T.

   Computing MIN  
   Writing concatenation */
    i__3[0] = 1, a__1[0] = side;
    i__3[1] = 1, a__1[1] = trans;
    s_cat(ch__1, a__1, i__3, &c__2, 2L);
    i__1 = 64, i__2 = ilaenv_(&c__1, "DORMRQ", ch__1, m, n, k, &c_n1, 6L, 2L);
    nb = min(i__1,i__2);
    nbmin = 2;
    ldwork = nw;
    if (nb > 1 && nb < *k) {
        iws = nw * nb;
        if (*lwork < iws) {
            nb = *lwork / ldwork;
/* Computing MAX  
   Writing concatenation */
            i__3[0] = 1, a__1[0] = side;
            i__3[1] = 1, a__1[1] = trans;
            s_cat(ch__1, a__1, i__3, &c__2, 2L);
            i__1 = 2, i__2 = ilaenv_(&c__2, "DORMRQ", ch__1, m, n, k, &c_n1,
                    6L, 2L);
            nbmin = max(i__1,i__2);
        }
    } else {
        iws = nw;
    }

    if (nb < nbmin || nb >= *k) {

/*        Use unblocked code */

        dormr2_(side, trans, m, n, k, &A(1,1), lda, &TAU(1), &C(1,1)
                , ldc, &WORK(1), &iinfo);
    } else {

/*        Use blocked code */

        if (left && ! notran || ! left && notran) {
            i1 = 1;
            i2 = *k;
            i3 = nb;
        } else {
            i1 = (*k - 1) / nb * nb + 1;
            i2 = 1;
            i3 = -nb;
        }

        if (left) {
            ni = *n;
        } else {
            mi = *m;
        }

        if (notran) {
            *(unsigned char *)transt = 'T';
        } else {
            *(unsigned char *)transt = 'N';
        }

        i__1 = i2;
        i__2 = i3;
        for (i = i1; i3 < 0 ? i >= i2 : i <= i2; i += i3) {
/* Computing MIN */
            i__4 = nb, i__5 = *k - i + 1;
            ib = min(i__4,i__5);

/*           Form the triangular factor of the block reflector
             H = H(i+ib-1) . . . H(i+1) H(i) */

            i__4 = nq - *k + i + ib - 1;
            dlarft_("Backward", "Rowwise", &i__4, &ib, &A(i,1), lda, &
                    TAU(i), t, &c__65);
            if (left) {

/*              H or H' is applied to C(1:m-k+i+ib-1,1:n) */

                mi = *m - *k + i + ib - 1;
            } else {

/*              H or H' is applied to C(1:m,1:n-k+i+ib-1) */

                ni = *n - *k + i + ib - 1;
            }

/*           Apply H or H' */

            dlarfb_(side, transt, "Backward", "Rowwise", &mi, &ni, &ib, &A(i,1),
 lda, t, &c__65, &C(1,1), ldc, &WORK(1), &
                    ldwork);
/* L10: */
        }
    }
    WORK(1) = (doublereal) iws;
    return 0;

/*     End of DORMRQ */

} /* dormrq_ */


#ifdef __cplusplus
}
#endif

