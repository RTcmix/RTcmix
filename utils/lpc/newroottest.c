/* newroottest.f -- translated by f2c (version of 26 January 1990  18:57:16).
   You must link the resulting object file with the libraries:
	-lF77 -lI77 -lm -lc   (in that order)
*/

#include "f2c.h"

/* Subroutine */ int stabletest_(frame, n, flag_)
real *frame;
integer *n;
integer *flag_;
{
    /* System generated locals */
    integer i_1, i_2;
    real r_1;

    /* Local variables */
    static real a[22500]	/* was [150][150] */;
    static integer i, m, mm;
    static real rk[249];
    /* Parameter adjustments */
    --frame;

    /* Function Body */
    *flag_ = 1;
    a[(*n + 1) * 150 - 150] = (float)1.;
    i_1 = *n;
    for (i = 1; i <= i_1; ++i) {
/* L10: */
	a[i + 1 + (*n + 1) * 150 - 151] = frame[i];
    }
    i_1 = *n;
    for (mm = 1; mm <= i_1; ++mm) {
	m = *n - mm + 1;
	rk[m - 1] = a[m + 1 + (m + 1) * 150 - 151];
	if ((r_1 = rk[m - 1], dabs(r_1)) < (float)1.) {
	    goto L20;
	}
	*flag_ = 0;
	return 0;
L20:
	i_2 = m;
	for (i = 1; i <= i_2; ++i) {
/* L25: */
/* Computing 2nd power */
	    r_1 = rk[m - 1];
	    a[i + m * 150 - 151] = (a[i + (m + 1) * 150 - 151] - rk[m - 1] * 
		    a[m - i + 2 + (m + 1) * 150 - 151]) / ((float)1. - r_1 * 
		    r_1);
	}
    }
    return 0;
} /* stable_ */

