/* lpc.roottest.f -- translated by f2c (version 19950110).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Table of constant values */

static integer c__1 = 1;

/* Subroutine */ int rootst_(anal, unstab, npoles, ifirst, ilast, anal_len, 
	unstab_len)
char *anal, *unstab;
integer *npoles, *ifirst, *ilast;
ftnlen anal_len;
ftnlen unstab_len;
{
    /* Format strings */
    static char fmt_145[] = "(a20,x,a20,x,i4,x,i4,x,i4)";

    /* System generated locals */
    integer i__1, i__2, i__3;
    olist o__1;

    /* Builtin functions */
    integer s_wsfe(), do_fio(), e_wsfe(), f_open(), s_rdue(), do_uio(), 
	    e_rdue(), s_wdue(), e_wdue();

    /* Local variables */
    static integer nbad;
    static logical flag__;
    static integer nall;
    static real a[40];
    static integer j, ndata;
    static real y[36];
    static integer nlocs, ii, jx;
    extern /* Subroutine */ int stable_();

    /* Fortran I/O blocks */
    static cilist io___4 = { 0, 6, 0, fmt_145, 0 };
    static cilist io___7 = { 0, 18, 1, 0, 0 };
    static cilist io___13 = { 0, 19, 0, 0, 0 };


    ndata = 4;
    nlocs = *npoles + ndata << 2;
    nall = *npoles + ndata;
/* L145: */
    s_wsfe(&io___4);
    do_fio(&c__1, anal, 64L);
    do_fio(&c__1, unstab, 64L);
    do_fio(&c__1, (char *)&(*npoles), (ftnlen)sizeof(integer));
    do_fio(&c__1, (char *)&(*ifirst), (ftnlen)sizeof(integer));
    do_fio(&c__1, (char *)&(*ilast), (ftnlen)sizeof(integer));
    e_wsfe();
    o__1.oerr = 0;
    o__1.ounit = 18;
    o__1.ofnmlen = 64;
    o__1.ofnm = anal;
    o__1.orl = nlocs;
    o__1.osta = "old";
    o__1.oacc = "direct";
    o__1.ofm = "unformatted";
    o__1.oblnk = 0;
    f_open(&o__1);
    o__1.oerr = 0;
    o__1.ounit = 19;
    o__1.ofnmlen = 64;
    o__1.ofnm = unstab;
    o__1.orl = 4;
    o__1.osta = "old";
    o__1.oacc = "direct";
    o__1.ofm = "unformatted";
    o__1.oblnk = 0;
    f_open(&o__1);
    nbad = 1;
    i__1 = *ilast;
    for (j = *ifirst; j <= i__1; ++j) {
	io___7.cirec = j;
	i__2 = s_rdue(&io___7);
	if (i__2 != 0) {
	    goto L999;
	}
	i__3 = nall;
	for (jx = 1; jx <= i__3; ++jx) {
	    i__2 = do_uio(&c__1, (char *)&a[jx - 1], (ftnlen)sizeof(real));
	    if (i__2 != 0) {
		goto L999;
	    }
	}
	i__2 = e_rdue();
	if (i__2 != 0) {
	    goto L999;
	}
	i__2 = *npoles;
	for (ii = 1; ii <= i__2; ++ii) {
	    y[ii - 1] = -(doublereal)a[nall + 1 - ii - 1];
/* L1601: */
	}
	stable_(y, npoles, &flag__);
	if (flag__) {
	    goto L1000;
	}
	io___13.cirec = nbad;
	s_wdue(&io___13);
	do_uio(&c__1, (char *)&j, (ftnlen)sizeof(integer));
	e_wdue();
	++nbad;
L1000:
	;
    }
L999:
    return 0;
} /* rootst_ */

/* Subroutine */ int stable_(frame, n, flag__)
real *frame;
integer *n;
logical *flag__;
{
    /* System generated locals */
    integer i__1, i__2;
    real r__1;

    /* Local variables */
    static real a[2500]	/* was [50][50] */;
    static integer i, m, mm;
    static real rk[49];

    /* Parameter adjustments */
    --frame;

    /* Function Body */
    *flag__ = TRUE_;
    a[(*n + 1) * 50 - 50] = (float)1.;
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
/* L10: */
	a[i + 1 + (*n + 1) * 50 - 51] = frame[i];
    }
    i__1 = *n;
    for (mm = 1; mm <= i__1; ++mm) {
	m = *n - mm + 1;
	rk[m - 1] = a[m + 1 + (m + 1) * 50 - 51];
	if ((r__1 = rk[m - 1], dabs(r__1)) < (float)1.) {
	    goto L20;
	}
	*flag__ = FALSE_;
	return 0;
L20:
	i__2 = m;
	for (i = 1; i <= i__2; ++i) {
/* L25: */
/* Computing 2nd power */
	    r__1 = rk[m - 1];
	    a[i + m * 50 - 51] = (a[i + (m + 1) * 50 - 51] - rk[m - 1] * a[m 
		    - i + 2 + (m + 1) * 50 - 51]) / ((float)1. - r__1 * r__1);
	}
    }
    return 0;
} /* stable_ */

