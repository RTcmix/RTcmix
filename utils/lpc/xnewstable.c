/* xnewstable.f -- translated by f2c (version 19950110).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Subroutine */ int correct_(frame, npoles, a)
doublereal *frame;
integer *npoles;
real *a;
{
    /* System generated locals */
    integer i__1, i__2, i__3, i__4, i__5;
    doublereal d__1, d__2;
    doublecomplex z__1, z__2;

    /* Builtin functions */
    double sqrt(), atan2(), cos(), sin();

    /* Local variables */
    static integer nall;
    static doublecomplex zero;
    static integer j, k;
    static doublereal r[37];
    static doublecomplex w[37];
    static integer ndata;
    static doublereal y[97], rooti[96];
    static integer l1, k4;
    static doublereal rootr[96];
    static integer ii;
    static doublereal th[37];
    static doublecomplex ww;
    extern /* Subroutine */ int factor_();
    static integer kinsid;
    static doublereal zz;
    static integer kprint, k4m;
    static doublecomplex one;
    static doublereal eps;

    /* Parameter adjustments */
    --a;
    --frame;

    /* Function Body */
    zero.r = 0., zero.i = 0.;
    one.r = 1., one.i = 0.;
    k4 = *npoles + 1;
    k4m = k4 - 1;
    nall = k4 + 4;
    i__1 = k4m;
    for (ii = 1; ii <= i__1; ++ii) {
/* L1601: */
	y[ii - 1] = -frame[ii + 4];
    }
    y[k4 - 1] = (float)1.;
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
	    z__2.r = ww.r * w[i__5].r - ww.i * w[i__5].i, z__2.i = ww.r * w[
		    i__5].i + ww.i * w[i__5].r;
	    z__1.r = w[i__4].r - z__2.r, z__1.i = w[i__4].i - z__2.i;
	    w[i__3].r = z__1.r, w[i__3].i = z__1.i;
	}
/* L20: */
	i__3 = k4 - 1;
	z__2.r = -ww.r, z__2.i = -ww.i;
	i__4 = k4 - 1;
	z__1.r = z__2.r * w[i__4].r - z__2.i * w[i__4].i, z__1.i = z__2.r * w[
		i__4].i + z__2.i * w[i__4].r;
	w[i__3].r = z__1.r, w[i__3].i = z__1.i;
    }
    i__3 = k4;
    for (j = 2; j <= i__3; ++j) {
	i__4 = j - 1;
	zz = w[i__4].r;
	a[k4 + ndata + 1 - j] = -zz;
/* L30: */
    }
    return 0;
} /* correct_ */

