/* newstable.f -- translated by f2c (version of 26 January 1990  18:57:16).
   You must link the resulting object file with the libraries:
	-lF77 -lI77 -lm -lc   (in that order)
*/

#include "f2c.h"

/* Subroutine */ int correct_(frame, npoles, a)
/*
doublereal *frame; 
*/
float *frame;
integer *npoles;
real *a;
{
    /* System generated locals */
    integer i,i_1, i_2, i_3, i_4, i_5;
    doublereal d_1, d_2;
    doublecomplex z_1, z_2;

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

/*
printf("\nin correct npoles = %d \n",*npoles);
for(i=0; i<36; i++) printf(" %g ",frame[i]);
*/
    /* Parameter adjustments */
    --frame; 
    --a;
    /* Function Body */
    zero.r = 0., zero.i = 0.;
    one.r = 1., one.i = 0.;
    k4 = *npoles + 1;
    k4m = k4 - 1;
    nall = k4 + 4;
    i_1 = k4m;
    for (ii = 1; ii <= i_1; ++ii) {
/* L1601: */
	y[ii - 1] = -frame[ii + 4];
    }
    y[k4 - 1] = (float)1.;
    eps = 1.0000000000000008e-8;
/* L303: */
    factor_(y, &k4, rootr, rooti, &kinsid, &kprint, &eps);
    i_1 = k4m;
    for (j = 1; j <= i_1; ++j) {
/* Computing 2nd power */
	d_1 = rootr[j - 1];
/* Computing 2nd power */
	d_2 = rooti[j - 1];
	r[j - 1] = sqrt(d_1 * d_1 + d_2 * d_2);
	th[j - 1] = atan2(rooti[j - 1], rootr[j - 1]);
	if (r[j - 1] >= 1.) {
	    r[j - 1] = (float)1. / r[j - 1];
	}
/* L100: */
    }
    i_1 = k4m;
    for (k = 1; k <= i_1; ++k) {
/* L10: */
	i_2 = k - 1;
	w[i_2].r = zero.r, w[i_2].i = zero.i;
    }
    i_2 = k4 - 1;
    w[i_2].r = one.r, w[i_2].i = one.i;
    i_2 = k4m;
    for (k = 1; k <= i_2; ++k) {
/* 	ww=dcmplx(rootr(k),rooti(k)) */
	d_1 = r[k - 1] * cos(th[k - 1]);
	d_2 = r[k - 1] * sin(th[k - 1]);
	z_1.r = d_1, z_1.i = d_2;
	ww.r = z_1.r, ww.i = z_1.i;
	l1 = k4 - k;
	i_1 = k4m;
	for (j = l1; j <= i_1; ++j) {
/* L12: */
	    i_3 = j - 1;
	    i_4 = j;
	    i_5 = j - 1;
	    z_2.r = ww.r * w[i_5].r - ww.i * w[i_5].i, z_2.i = ww.r * w[i_5]
		    .i + ww.i * w[i_5].r;
	    z_1.r = w[i_4].r - z_2.r, z_1.i = w[i_4].i - z_2.i;
	    w[i_3].r = z_1.r, w[i_3].i = z_1.i;
	}
/* L20: */
	i_3 = k4 - 1;
	z_2.r = -ww.r, z_2.i = -ww.i;
	i_4 = k4 - 1;
	z_1.r = z_2.r * w[i_4].r - z_2.i * w[i_4].i, z_1.i = z_2.r * w[i_4].i 
		+ z_2.i * w[i_4].r;
	w[i_3].r = z_1.r, w[i_3].i = z_1.i;
    }
    i_3 = k4;
    for (j = 2; j <= i_3; ++j) {
	i_4 = j - 1;
	zz = w[i_4].r;
	a[k4 + ndata + 1 - j] = -zz;
/* L30: */
    }
    return 0;
} /* correct_ */

