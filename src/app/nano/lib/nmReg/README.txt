Author: Adam Seeger

Since I'm only using two functions from CLAPACK, I thought it would be
bad to have to compile and link with the whole CLAPACK, BLAS, and F2C libraries
so I extracted the files we really need to minimize weight gain of microscape.
I also added extern "C" declarations so we can compile with CC.
I also edited the files to remove the tons of compiler warnings that are
the result of the f2c conversion. 

/**  barf  [ba:rf]  2.  "He suggested using FORTRAN, and everybody barfed."

    - From The Shogakukan DICTIONARY OF NEW ENGLISH (Second edition) */

The complete CLAPACK package is available from www.netlib.org.

dgels.c - contains the dgels.c file from CLAPACK/SRC plus files it depends on
	from CLAPACK/SRC:
	dgels.c (original)
	dgelq2.c
	dgelqf.c
	dgeqr2.c
	dgeqrf.c
	dlabad.c
	dlamch.c
	dlamc1.c
	dlamc2.c
	dlamc3.c
	dlamc4.c
	dlamc5.c
	dlange.c
	dlapy2.c
	dlarf.c
	dlarfb.c
	dlarfg.c
	dlarft.c
	dlascl.c
	dlaset.c
	dlassq.c
	dorm2r.c
	dorml2.c
	dormlq.c
	dormqr.c
	ilaenv.c
	lsame.c
	xerbla.c

dgglse.c - contains the dgglse.c file from CLAPACK/SRC plus files it depends
	on from CLAPACK/SRC minus files it depends on that are in the list
	above:
	dgglse.c (original)
	dgerq2.c
	dgerqf.c
	dggrqf.c
	dormr2.c
	dormrq.c

blas_extract.c - contains files that dgels.c depends on from 
	CLAPACK/BLAS/SRC:
	dtrsm.c
	dgemv.c
	dtrmm.c
	dgemm.c
	dtrmv.c
	dcopy.c
	dgemm.c
	dtrmv.c
	dcopy.c
	dnrm2.c
	dscal.c
	dger.c

f2c_extract.c - contains files that dgels.c depends on from
	CLAPACK/F2CLIBS/libF77:
	d_lg10.c
	pow_di.c
	d_sign.c
	s_cat.c
	s_copy.c
	s_cmp.c
