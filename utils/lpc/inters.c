/* inters.f -- translated by f2c (version of 26 January 1990  18:57:16).
   You must link the resulting object file with the libraries:
        -lF77 -lI77 -lm -lc   (in that order)
*/

#include "f2c.h"

/* Subroutine */ int inters_(bold, bnew, deltb, npols, b)
real *bold, *bnew;
real *deltb;
integer *npols;
real *b;
{
    /* System generated locals */
    integer i_1, i_2;
    doublereal d_1;

    /* Local variables */
    static doublereal aold[576] /* was [24][24] */, anew[576]   /* was [24][
            24] */, rold[25], rnew[25];
    static integer ncom1, ncom2;
    static doublereal a[576]    /* was [24][24] */;
    static integer i, j, k;
    static doublereal r[25], delta;
    static integer iminus, npolsm, npolsp;
    static real sum1, sum2;

    /* Parameter adjustments */
    --bold;
    --bnew;
    --b;

    /* Function Body */
    delta = *deltb;
    ncom1 = *npols + 1;
    i_1 = *npols;
    for (i = 1; i <= i_1; ++i) {
        ncom2 = ncom1 - i;
        aold[*npols + i * 24 - 25] = bold[ncom2];
/* L10: */
        anew[*npols + i * 24 - 25] = bnew[ncom2];
    }
    npolsm = *npols - 1;
    npolsp = *npols + 1;
    i_1 = npolsm;
    for (k = 1; k <= i_1; ++k) {
        i = *npols - k;
        i_2 = i;
        for (j = 1; j <= i_2; ++j) {
/* Computing 2nd power */
            d_1 = aold[i + 1 + (i + 1) * 24 - 25];
            aold[i + j * 24 - 25] = (aold[i + 1 + j * 24 - 25] + aold[i + 1 + 
                    (i + 1) * 24 - 25] * aold[i + 1 + (i + 1 - j) * 24 - 25]) 
                    / ((float)1. - d_1 * d_1);
/* L20: */
/* Computing 2nd power */
            d_1 = anew[i + 1 + (i + 1) * 24 - 25];
            anew[i + j * 24 - 25] = (anew[i + 1 + j * 24 - 25] + anew[i + 1 + 
                    (i + 1) * 24 - 25] * anew[i + 1 + (i + 1 - j) * 24 - 25]) 
                    / ((float)1. - d_1 * d_1);
        }
    }
/* ......COMPUTE AUTOCORRELATION COEFFICIENTS RTEMP(I) WITH R(0) = 1 */
    rold[0] = (float)1.;
    rnew[0] = (float)1.;
    i_2 = *npols;
    for (i = 1; i <= i_2; ++i) {
        sum1 = (float)0.;
        sum2 = (float)0.;
        i_1 = i;
        for (j = 1; j <= i_1; ++j) {
            sum1 += aold[i + j * 24 - 25] * rold[i + 1 - j - 1];
/* L40: */
            sum2 += anew[i + j * 24 - 25] * rnew[i + 1 - j - 1];
        }
        rold[i] = sum1;
/* L30: */
        rnew[i] = sum2;
    }
/* ......COMPUTE R(0) FROM RTEMP(I) */
    i_2 = *npols;
    for (i = 1; i <= i_2; ++i) {
        rold[0] -= aold[*npols + i * 24 - 25] * rold[i];
/* L60: */
        rnew[0] -= anew[*npols + i * 24 - 25] * rnew[i];
    }
    rold[0] = 1. / rold[0];
    rnew[0] = 1. / rnew[0];
/* ......COMPUTE AUTOCORRELATION COEFFICIENTS R(I) WITH CORRECT R(0) */
    i_2 = npolsp;
    for (i = 2; i <= i_2; ++i) {
        rold[i - 1] = rold[0] * rold[i - 1];
/* L50: */
        rnew[i - 1] = rnew[0] * rnew[i - 1];
    }
/* ......INTERPOLATE AUTOCORRELATION COEFFICIENTS */
    i_2 = npolsp;
    for (i = 1; i <= i_2; ++i) {
/* L70: */
        r[i - 1] = delta * (rnew[i - 1] - rold[i - 1]) + rold[i - 1];
    }
/* ......INSERT TEST FOR SINGULARITY OF NEW AUTOCORRELATION MATRIX */
/* ......COMPUTE NEW PREDICTOR COEFFICIENTS */
    a[0] = r[1] / r[0];
    i_2 = *npols;
    for (i = 2; i <= i_2; ++i) {
        sum1 = r[0];
        sum2 = r[i];
        iminus = i - 1;
        i_1 = iminus;
        for (j = 1; j <= i_1; ++j) {
            sum1 -= r[j] * a[i - 1 + j * 24 - 25];
/* L90: */
            sum2 -= r[i - j] * a[i - 1 + j * 24 - 25];
        }
        a[i + i * 24 - 25] = sum2 / sum1;
        i_1 = iminus;
        for (j = 1; j <= i_1; ++j) {
/* L100: */
            a[i + j * 24 - 25] = a[i - 1 + j * 24 - 25] - a[i + i * 24 - 25] *
                     a[i - 1 + (i - j) * 24 - 25];
        }
/* L80: */
    }
    i_2 = *npols;
    for (i = 1; i <= i_2; ++i) {
        ncom2 = ncom1 - i;
/* L110: */
        b[i] = a[*npols + ncom2 * 24 - 25];
    }
    return 0;
} /* inters_ */
