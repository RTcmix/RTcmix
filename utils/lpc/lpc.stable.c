/* lpc.stable.f -- translated by f2c (version 19950110).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Table of constant values */

static integer c__1 = 1;

/* Subroutine */ int stabl_(anal, unstab, npoles, anal_len, unstab_len)
char *anal, *unstab;
integer *npoles;
ftnlen anal_len;
ftnlen unstab_len;
{
    /* System generated locals */
    integer i__1, i__2, i__3, i__4, i__5;
    doublereal d__1, d__2;
    doublecomplex z__1, z__2;
    olist o__1;

    /* Builtin functions */
    integer f_open(), s_rdue(), do_uio(), e_rdue();
    double sqrt(), atan2(), cos(), sin();
    integer s_wdue(), e_wdue();
    /* Subroutine */ int s_stop();

    /* Local variables */
    static integer nall;
    static doublecomplex zero;
    static real a[40];
    static integer j, k;
    static doublereal r[37];
    static doublecomplex w[37];
    static integer ndata;
    static doublereal y[37];
    static integer nlocs;
    static doublereal rooti[36];
    static integer l1, k4;
    static doublereal rootr[36];
    static integer ii, ll;
    static doublereal th[37];
    static doublecomplex ww;
    extern /* Subroutine */ int factor_();
    static integer kinsid;
    static doublereal zz;
    static integer kprint, k4m, jjj, kkk;
    static doublecomplex one;
    static doublereal eps;

    /* Fortran I/O blocks */
    static cilist io___9 = { 0, 19, 1, 0, 0 };
    static cilist io___11 = { 0, 18, 1, 0, 0 };
    static cilist io___29 = { 0, 18, 0, 0, 0 };


    zero.r = 0., zero.i = 0.;
    one.r = 1., one.i = 0.;
    ndata = 4;
    nlocs = *npoles + ndata << 2;
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
    k4 = *npoles + 1;
    k4m = k4 - 1;
    nall = k4m + ndata;
    for (jjj = 1; jjj <= 10000; ++jjj) {
	io___9.cirec = jjj;
	i__1 = s_rdue(&io___9);
	if (i__1 != 0) {
	    goto L999;
	}
	i__1 = do_uio(&c__1, (char *)&kkk, (ftnlen)sizeof(integer));
	if (i__1 != 0) {
	    goto L999;
	}
	i__1 = e_rdue();
	if (i__1 != 0) {
	    goto L999;
	}
	io___11.cirec = kkk;
	i__1 = s_rdue(&io___11);
	if (i__1 != 0) {
	    goto L999;
	}
	i__2 = nall;
	for (ll = 1; ll <= i__2; ++ll) {
	    i__1 = do_uio(&c__1, (char *)&a[ll - 1], (ftnlen)sizeof(real));
	    if (i__1 != 0) {
		goto L999;
	    }
	}
	i__1 = e_rdue();
	if (i__1 != 0) {
	    goto L999;
	}
	i__1 = k4m;
	for (ii = 1; ii <= i__1; ++ii) {
/* L1601: */
	    y[ii - 1] = -(doublereal)a[ii + 3];
	}
	y[k4 - 1] = (float)1.;
	kprint = 0;
	eps = 1.0000000000000008e-8;
/* L303: */
	factor_(y, &k4, rootr, rooti, &kinsid, &kprint, &eps);
	i__1 = k4m;
	for (j = 1; j <= i__1; ++j) {
/* Computing 2nd power */
	    d__1 = rootr[j - 1];
/* Computing 2nd power */
	    d__2 = rooti[j - 1];
	    r[j - 1] = sqrt(d__1 * d__1 + d__2 * d__2);
	    th[j - 1] = atan2(rooti[j - 1], rootr[j - 1]);
	    if (r[j - 1] >= (float)1.) {
		r[j - 1] = (float)1. / r[j - 1];
	    }
/* L100: */
	}
	i__1 = k4m;
	for (k = 1; k <= i__1; ++k) {
/* L10: */
	    i__2 = k - 1;
	    w[i__2].r = zero.r, w[i__2].i = zero.i;
	}
	i__2 = k4 - 1;
	w[i__2].r = one.r, w[i__2].i = one.i;
	i__2 = k4m;
	for (k = 1; k <= i__2; ++k) {
/* 	ww=dcmplx(rootr(k),rooti(k)) */
	    d__1 = r[k - 1] * cos(th[k - 1]);
	    d__2 = r[k - 1] * sin(th[k - 1]);
	    z__1.r = d__1, z__1.i = d__2;
	    ww.r = z__1.r, ww.i = z__1.i;
	    l1 = k4 - k;
	    i__1 = k4m;
	    for (j = l1; j <= i__1; ++j) {
/* L12: */
		i__3 = j - 1;
		i__4 = j;
		i__5 = j - 1;
		z__2.r = ww.r * w[i__5].r - ww.i * w[i__5].i, z__2.i = ww.r * 
			w[i__5].i + ww.i * w[i__5].r;
		z__1.r = w[i__4].r - z__2.r, z__1.i = w[i__4].i - z__2.i;
		w[i__3].r = z__1.r, w[i__3].i = z__1.i;
	    }
/* L20: */
	    i__3 = k4 - 1;
	    z__2.r = -ww.r, z__2.i = -ww.i;
	    i__4 = k4 - 1;
	    z__1.r = z__2.r * w[i__4].r - z__2.i * w[i__4].i, z__1.i = z__2.r 
		    * w[i__4].i + z__2.i * w[i__4].r;
	    w[i__3].r = z__1.r, w[i__3].i = z__1.i;
	}
	i__3 = k4;
	for (j = 2; j <= i__3; ++j) {
	    i__4 = j - 1;
	    zz = w[i__4].r;
	    a[k4 + ndata + 1 - j - 1] = -zz;
/* L30: */
	}
	io___29.cirec = kkk;
	s_wdue(&io___29);
	i__3 = nall;
	for (ll = 1; ll <= i__3; ++ll) {
	    do_uio(&c__1, (char *)&a[ll - 1], (ftnlen)sizeof(real));
	}
	e_wdue();
/* L199: */
    }
L999:
    s_stop("", 0L);
} /* stabl_ */

