/* factor.f -- translated by f2c (version of 26 January 1990  18:57:16).
   You must link the resulting object file with the libraries:
	-lF77 -lI77 -lm -lc   (in that order)
*/

#include "f2c.h"

/* Subroutine */ int factor_(b, k4, rootr, rooti, kinsid, kprint, eps)
doublereal *b;
integer *k4;
doublereal *rootr, *rooti;
integer *kinsid, *kprint;
doublereal *eps;
{
    /* System generated locals */
    integer i_1, i_2, i_3;
    doublereal d_1, d_2;
    doublecomplex z_1, z_2;

    /* Builtin functions */
    double atan2();
    /* Subroutine */ int s_stop();
    double pow_dd(), sqrt();

    /* Local variables */
    static doublereal amax;
    static integer kerr;
    static doublereal dist, rmin, rmax;
    static integer i, j, k;
    static doublereal r;
    static doublecomplex z;
    static doublereal parti, distr, partr, r2, pi, resmag, resmax;
    extern /* Subroutine */ int dproot_();
    static integer k4m;
    static doublereal coe[37];
    static doublecomplex jay, res;

/*
printf("in factornpoles = %d\n",*k4);
for(i=0; i<36; i++) printf(" %g ",b[i]);
*/
    /* Parameter adjustments */
    --b;
    --rootr;
    --rooti;
    /* Function Body */
/*       sets up problem, calls dproot, */
/*       and checks residual values at roots */
    jay.r = 0., jay.i = 1.;
    pi = atan2(1., 1.) * 4.;
    i_1 = *k4;
    for (i = 1; i <= i_1; ++i) {
/* L550: */
	coe[i - 1] = b[i];
    }
    k4m = *k4 - 1;
    dproot_(&k4m, coe, &rootr[1], &rooti[1], &kerr, kprint, eps);
/*      write(0,600)kerr */
/* L600: */
    if (kerr > 0) {
	s_stop("", 0L);
    }
    *kinsid = 0;
    resmax = 0.;
    rmax = 0.;
    rmin = 4294967296.;
    dist = 4294967296.;
    amax = 4294967296.;
    d_1 = 1. / *k4;
    r2 = pow_dd(&amax, &d_1);
    i_1 = k4m;
    for (j = 1; j <= i_1; ++j) {
	i_2 = j;
	i_3 = j;
	z_2.r = rooti[i_3] * jay.r, z_2.i = rooti[i_3] * jay.i;
	z_1.r = rootr[i_2] + z_2.r, z_1.i = z_2.i;
	z.r = z_1.r, z.i = z_1.i;
/* Computing 2nd power */
	d_1 = rootr[j];
/* Computing 2nd power */
	d_2 = rooti[j];
	r = sqrt(d_1 * d_1 + d_2 * d_2);
/*         skip residue calculation if root is too big */
	if (r > r2) {
	    goto L713;
	}
	i_2 = *k4;
	res.r = b[i_2], res.i = 0.;
	i_2 = *k4;
	for (k = 2; k <= i_2; ++k) {
/* L705: */
	    z_2.r = res.r * z.r - res.i * z.i, z_2.i = res.r * z.i + res.i * 
		    z.r;
	    i_3 = *k4 - k + 1;
	    z_1.r = z_2.r + b[i_3], z_1.i = z_2.i;
	    res.r = z_1.r, res.i = z_1.i;
	}
	partr = res.r;
	z_2.r = -jay.r, z_2.i = -jay.i;
	z_1.r = z_2.r * res.r - z_2.i * res.i, z_1.i = z_2.r * res.i + z_2.i *
		 res.r;
	parti = z_1.r;
/* Computing 2nd power */
	d_1 = partr;
/* Computing 2nd power */
	d_2 = parti;
	resmag = sqrt(d_1 * d_1 + d_2 * d_2);
	if (resmax <= resmag) {
	    resmax = resmag;
	}
L713:
	if (rmax < r) {
	    rmax = r;
	}
	if (rmin > r) {
	    rmin = r;
	}
	if (r < 1.) {
	    ++(*kinsid);
	}
	distr = (d_1 = r - 1., abs(d_1));
	if (dist > distr) {
	    dist = distr;
	}
/* L701: */
    }
/*     write(0,703)resmax */
/*     write(0,704)rmax,rmin,dist */
/* 703   format('resmax= ',d20.10) */
/* 704   format('rmax= ',d20.10/'rmin= ',d20.10/'dist=',d20.10) */
    return 0;
} /* factor_ */

/* Subroutine */ int dproot_(mm, a, rootr, rooti, kerr, kprint, eps)
integer *mm;
doublereal *a, *rootr, *rooti;
integer *kerr, *kprint;
doublereal *eps;
{
    /* System generated locals */
    integer i_1, i_2, i_3, i_4;
    doublereal d_1, d_2;
    doublecomplex z_1, z_2, z_3;

    /* Builtin functions */
    double pow_dd(), sqrt(), cos(), sin();
    void z_div();

    /* Local variables */
    static integer mdec;
    static doublereal amin, amax, save[37];
    static integer kmax, kpol;
    static doublereal temp, size;
    static integer ktry;
    static doublereal real1, real2;
    static doublecomplex b[37], c[37];
    static integer i, j, k, m;
    static doublecomplex p, w, z;
    static doublereal parti;
    static integer kpolm;
    static doublereal partr;
    static integer kount, newst, nroot;
    static doublereal r1, r2;
    static integer ktrym;
    static doublecomplex bb[37], cc[37];
    static integer mp;
    static doublecomplex pp;
    static doublereal sqteps, rkount, rr1, rr2;
    static doublecomplex jay;
    static integer mmp;

    /* Parameter adjustments */
    --a;
    --rootr;
    --rooti;

    /* Function Body */
/*        mm=degree of polynomial */
/*        a=coefficient array, lowest to highest degree */
/*        kprint=1 for full printing */
/*        kerr=0 is normal return */
    jay.r = 0., jay.i = 1.;
    mmp = *mm + 1;
    m = *mm;
    mp = mmp;
    i_1 = mp;
    for (i = 1; i <= i_1; ++i) {
/* L700: */
	save[i - 1] = a[i];
    }
/*       kount is number of iterations so far */
    kount = 0;
/*       kmax is maximum total number of iterations allowed */
    kmax = m * 20;
/*       newst is number of re-starts */
    newst = 0;
/*       ktrym is number of attempted iterations before re-starting */
    ktrym = 20;
/*      kpolm is number of attempted iterations before polishing is 
stopped*/
    kpolm = 20;
/*       amax is the largest number we allow */
    amax = 4294967296.;
    amin = 1. / amax;
/*       rr1 and rr2 are radii within which we work for polishing */
    d_1 = 1. / m;
    rr1 = pow_dd(&amin, &d_1);
    d_1 = 1. / m;
    rr2 = pow_dd(&amax, &d_1);
/*     eps is the tolerance for convergence */
    sqteps = sqrt(*eps);
/*        main loop; m is current degree */
L10:
    if (m <= 0) {
	goto L200;
    }
/*        new z, a point on the unit circle */
    rkount = (doublereal) kount;
    d_1 = cos(rkount);
    d_2 = sin(rkount);
    z_2.r = d_2 * jay.r, z_2.i = d_2 * jay.i;
    z_1.r = d_1 + z_2.r, z_1.i = z_2.i;
    z.r = z_1.r, z.i = z_1.i;
    ktry = 0;
/*      r1 and r2 are boundaries of an expanding annulus within which we 
work*/
    d_1 = 1. / m;
    r1 = pow_dd(&amin, &d_1);
    d_1 = 1. / m;
    r2 = pow_dd(&amax, &d_1);
/*        inside loop */
L20:
    partr = z.r;
    z_2.r = -jay.r, z_2.i = -jay.i;
    z_1.r = z_2.r * z.r - z_2.i * z.i, z_1.i = z_2.r * z.i + z_2.i * z.r;
    parti = z_1.r;
/* Computing 2nd power */
    d_1 = partr;
/* Computing 2nd power */
    d_2 = parti;
    size = sqrt(d_1 * d_1 + d_2 * d_2);
    if (size < r1 || size > r2) {
	goto L300;
    }
    if (ktry >= ktrym) {
	goto L300;
    }
    ++ktry;
    if (kount >= kmax) {
	goto L400;
    }
    ++kount;
/*        get value of polynomial at z, synthetic division */
    i_1 = mp - 1;
    i_2 = mp;
    b[i_1].r = a[i_2], b[i_1].i = 0.;
    i_1 = m;
    for (j = 1; j <= i_1; ++j) {
	k = m - j + 1;
/* L30: */
	i_2 = k - 1;
	i_3 = k;
	z_2.r = z.r * b[i_3].r - z.i * b[i_3].i, z_2.i = z.r * b[i_3].i + z.i 
		* b[i_3].r;
	i_4 = k;
	z_1.r = z_2.r + a[i_4], z_1.i = z_2.i;
	b[i_2].r = z_1.r, b[i_2].i = z_1.i;
    }
    p.r = b[0].r, p.i = b[0].i;
    partr = p.r;
    z_2.r = -jay.r, z_2.i = -jay.i;
    z_1.r = z_2.r * p.r - z_2.i * p.i, z_1.i = z_2.r * p.i + z_2.i * p.r;
    parti = z_1.r;
/* Computing 2nd power */
    d_1 = partr;
/* Computing 2nd power */
    d_2 = parti;
    if (sqrt(d_1 * d_1 + d_2 * d_2) > amax) {
	goto L300;
    }
/*        get value of derivative at z, synthetic division */
    i_2 = mp - 1;
    i_3 = mp - 1;
    c[i_2].r = b[i_3].r, c[i_2].i = b[i_3].i;
    mdec = m - 1;
    i_2 = mdec;
    for (j = 1; j <= i_2; ++j) {
	k = m - j + 1;
/* L60: */
	i_3 = k - 1;
	i_4 = k;
	z_2.r = z.r * c[i_4].r - z.i * c[i_4].i, z_2.i = z.r * c[i_4].i + z.i 
		* c[i_4].r;
	i_1 = k - 1;
	z_1.r = z_2.r + b[i_1].r, z_1.i = z_2.i + b[i_1].i;
	c[i_3].r = z_1.r, c[i_3].i = z_1.i;
    }
    pp.r = c[1].r, pp.i = c[1].i;
    partr = pp.r;
    z_2.r = -jay.r, z_2.i = -jay.i;
    z_1.r = z_2.r * pp.r - z_2.i * pp.i, z_1.i = z_2.r * pp.i + z_2.i * pp.r;
    parti = z_1.r;
/* Computing 2nd power */
    d_1 = partr;
/* Computing 2nd power */
    d_2 = parti;
    if (sqrt(d_1 * d_1 + d_2 * d_2) < amin) {
	goto L300;
    }
/*        test for convergence */
    partr = p.r;
    z_2.r = -jay.r, z_2.i = -jay.i;
    z_1.r = z_2.r * p.r - z_2.i * p.i, z_1.i = z_2.r * p.i + z_2.i * p.r;
    parti = z_1.r;
/* Computing 2nd power */
    d_1 = partr;
/* Computing 2nd power */
    d_2 = parti;
    size = sqrt(d_1 * d_1 + d_2 * d_2);
    if (size > *eps) {
	goto L775;
    }
    nroot = *mm - m + 1;
    goto L500;
L775:
    z_div(&z_2, &p, &pp);
    z_1.r = z.r - z_2.r, z_1.i = z.i - z_2.i;
    z.r = z_1.r, z.i = z_1.i;
    goto L20;
/*        end of main loop */
/*        normal return */
L200:
    *kerr = 0;
    goto L600;
/*        new start */
L300:
    rkount = (doublereal) kount;
    d_1 = cos(rkount);
    d_2 = sin(rkount);
    z_2.r = d_2 * jay.r, z_2.i = d_2 * jay.i;
    z_1.r = d_1 + z_2.r, z_1.i = z_2.i;
    z.r = z_1.r, z.i = z_1.i;
    ktry = 0;
    ++newst;
    goto L20;
/*        too many iterations */
L400:
    *kerr = 400;
    goto L600;
/*        root z located */
/*        polish z to get w */
L500:
    w.r = z.r, w.i = z.i;
    kpol = 0;
L510:
    partr = w.r;
    z_2.r = -jay.r, z_2.i = -jay.i;
    z_1.r = z_2.r * w.r - z_2.i * w.i, z_1.i = z_2.r * w.i + z_2.i * w.r;
    parti = z_1.r;
/* Computing 2nd power */
    d_1 = partr;
/* Computing 2nd power */
    d_2 = parti;
    size = sqrt(d_1 * d_1 + d_2 * d_2);
/*       give up polishing if w is outside annulus */
    if (size < rr1 || size > rr2) {
	goto L501;
    }
/*       give up polishing if kpol>=kpolm */
    if (kpol >= kpolm) {
	goto L501;
    }
    ++kpol;
    if (kount >= kmax) {
	goto L400;
    }
    ++kount;
    i_3 = mmp - 1;
    i_4 = mmp - 1;
    bb[i_3].r = save[i_4], bb[i_3].i = 0.;
    i_3 = *mm;
    for (j = 1; j <= i_3; ++j) {
	k = *mm - j + 1;
/* L530: */
	i_4 = k - 1;
	i_1 = k;
	z_2.r = w.r * bb[i_1].r - w.i * bb[i_1].i, z_2.i = w.r * bb[i_1].i + 
		w.i * bb[i_1].r;
	i_2 = k - 1;
	z_1.r = z_2.r + save[i_2], z_1.i = z_2.i;
	bb[i_4].r = z_1.r, bb[i_4].i = z_1.i;
    }
    p.r = bb[0].r, p.i = bb[0].i;
    partr = p.r;
    z_2.r = -jay.r, z_2.i = -jay.i;
    z_1.r = z_2.r * p.r - z_2.i * p.i, z_1.i = z_2.r * p.i + z_2.i * p.r;
    parti = z_1.r;
/* Computing 2nd power */
    d_1 = partr;
/* Computing 2nd power */
    d_2 = parti;
    if (sqrt(d_1 * d_1 + d_2 * d_2) > amax) {
	goto L300;
    }
    i_4 = mmp - 1;
    i_1 = mmp - 1;
    cc[i_4].r = bb[i_1].r, cc[i_4].i = bb[i_1].i;
    mdec = *mm - 1;
    i_4 = mdec;
    for (j = 1; j <= i_4; ++j) {
	k = *mm - j + 1;
/* L560: */
	i_1 = k - 1;
	i_2 = k;
	z_2.r = w.r * cc[i_2].r - w.i * cc[i_2].i, z_2.i = w.r * cc[i_2].i + 
		w.i * cc[i_2].r;
	i_3 = k - 1;
	z_1.r = z_2.r + bb[i_3].r, z_1.i = z_2.i + bb[i_3].i;
	cc[i_1].r = z_1.r, cc[i_1].i = z_1.i;
    }
    pp.r = cc[1].r, pp.i = cc[1].i;
    partr = pp.r;
    z_2.r = -jay.r, z_2.i = -jay.i;
    z_1.r = z_2.r * pp.r - z_2.i * pp.i, z_1.i = z_2.r * pp.i + z_2.i * pp.r;
    parti = z_1.r;
/* Computing 2nd power */
    d_1 = partr;
/* Computing 2nd power */
    d_2 = parti;
    if (sqrt(d_1 * d_1 + d_2 * d_2) < amin) {
	goto L300;
    }
    partr = p.r;
    z_2.r = -jay.r, z_2.i = -jay.i;
    z_1.r = z_2.r * p.r - z_2.i * p.i, z_1.i = z_2.r * p.i + z_2.i * p.r;
    parti = z_1.r;
/* Computing 2nd power */
    d_1 = partr;
/* Computing 2nd power */
    d_2 = parti;
    size = sqrt(d_1 * d_1 + d_2 * d_2);
/*       test for convergence of polishing */
    if (size <= *eps) {
	goto L501;
    }
    z_div(&z_2, &p, &pp);
    z_1.r = w.r - z_2.r, z_1.i = w.i - z_2.i;
    w.r = z_1.r, w.i = z_1.i;
    goto L510;
/*        deflate */
L501:
    i_1 = mp - 1;
    i_2 = mp;
    b[i_1].r = a[i_2], b[i_1].i = 0.;
    i_1 = m;
    for (j = 1; j <= i_1; ++j) {
	k = m - j + 1;
/* L830: */
	i_2 = k - 1;
	i_3 = k;
	z_2.r = z.r * b[i_3].r - z.i * b[i_3].i, z_2.i = z.r * b[i_3].i + z.i 
		* b[i_3].r;
	i_4 = k;
	z_1.r = z_2.r + a[i_4], z_1.i = z_2.i;
	b[i_2].r = z_1.r, b[i_2].i = z_1.i;
    }
    p.r = b[0].r, p.i = b[0].i;
    i_2 = m;
    rootr[i_2] = w.r;
    i_2 = m;
    z_2.r = -jay.r, z_2.i = -jay.i;
    z_1.r = z_2.r * w.r - z_2.i * w.i, z_1.i = z_2.r * w.i + z_2.i * w.r;
    rooti[i_2] = z_1.r;
    --m;
    --mp;
    z_2.r = -jay.r, z_2.i = -jay.i;
    z_1.r = z_2.r * w.r - z_2.i * w.i, z_1.i = z_2.r * w.i + z_2.i * w.r;
    parti = z_1.r;
    if (abs(parti) > sqteps) {
	goto L140;
    }
/*        real root */
    rooti[m + 1] = 0.;
    i_2 = mp;
    for (j = 1; j <= i_2; ++j) {
/* L100: */
	i_3 = j;
	i_4 = j;
	a[i_3] = b[i_4].r;
    }
    goto L10;
/*        complex root */
L140:
    partr = z.r;
    z_2.r = -jay.r, z_2.i = -jay.i;
    z_1.r = z_2.r * z.r - z_2.i * z.i, z_1.i = z_2.r * z.i + z_2.i * z.r;
    parti = z_1.r;
    z_2.r = parti * jay.r, z_2.i = parti * jay.i;
    z_1.r = partr - z_2.r, z_1.i = -z_2.i;
    z.r = z_1.r, z.i = z_1.i;
    i_3 = mp - 1;
    i_4 = mp;
    c[i_3].r = b[i_4].r, c[i_3].i = b[i_4].i;
    i_3 = m;
    for (j = 1; j <= i_3; ++j) {
	k = m - j + 1;
/* L110: */
	i_4 = k - 1;
	i_2 = k;
	z_2.r = z.r * c[i_2].r - z.i * c[i_2].i, z_2.i = z.r * c[i_2].i + z.i 
		* c[i_2].r;
	i_1 = k;
	z_1.r = z_2.r + b[i_1].r, z_1.i = z_2.i + b[i_1].i;
	c[i_4].r = z_1.r, c[i_4].i = z_1.i;
    }
    i_4 = m;
    rootr[i_4] = w.r;
    i_4 = m;
    z_3.r = -jay.r, z_3.i = -jay.i;
    z_2.r = z_3.r * w.r - z_3.i * w.i, z_2.i = z_3.r * w.i + z_3.i * w.r;
    z_1.r = -z_2.r, z_1.i = -z_2.i;
    rooti[i_4] = z_1.r;
    --m;
    --mp;
    i_4 = mp;
    for (j = 1; j <= i_4; ++j) {
/* L130: */
	i_2 = j;
	i_1 = j;
	a[i_2] = c[i_1].r;
    }
    goto L10;
/*        report and return */
L600:
    real1 = (doublereal) kount;
    real2 = (doublereal) (*mm);
    temp = real1 / real2;
/*     write(0,150)kount,temp */
/* L150: */
/*     write(0,151)newst,kerr */
/* L151: */
    return 0;
} /* dproot_ */

