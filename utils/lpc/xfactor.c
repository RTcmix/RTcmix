/* xfactor.f -- translated by f2c (version 19950110).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
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
    integer i__1, i__2, i__3;
    doublereal d__1, d__2;
    doublecomplex z__1, z__2;

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

/*       sets up problem, calls dproot, */
/*       and checks residual values at roots */
    /* Parameter adjustments */
    --rooti;
    --rootr;
    --b;

    /* Function Body */
    jay.r = 0., jay.i = 1.;
    pi = atan2(1., 1.) * 4.;
    i__1 = *k4;
    for (i = 1; i <= i__1; ++i) {
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
    d__1 = 1. / *k4;
    r2 = pow_dd(&amax, &d__1);
    i__1 = k4m;
    for (j = 1; j <= i__1; ++j) {
	i__2 = j;
	i__3 = j;
	z__2.r = rooti[i__3] * jay.r, z__2.i = rooti[i__3] * jay.i;
	z__1.r = rootr[i__2] + z__2.r, z__1.i = z__2.i;
	z.r = z__1.r, z.i = z__1.i;
/* Computing 2nd power */
	d__1 = rootr[j];
/* Computing 2nd power */
	d__2 = rooti[j];
	r = sqrt(d__1 * d__1 + d__2 * d__2);
/*         skip residue calculation if root is too big */
	if (r > r2) {
	    goto L713;
	}
	i__2 = *k4;
	res.r = b[i__2], res.i = 0.;
	i__2 = *k4;
	for (k = 2; k <= i__2; ++k) {
/* L705: */
	    z__2.r = res.r * z.r - res.i * z.i, z__2.i = res.r * z.i + res.i *
		     z.r;
	    i__3 = *k4 - k + 1;
	    z__1.r = z__2.r + b[i__3], z__1.i = z__2.i;
	    res.r = z__1.r, res.i = z__1.i;
	}
	partr = res.r;
	z__2.r = -jay.r, z__2.i = -jay.i;
	z__1.r = z__2.r * res.r - z__2.i * res.i, z__1.i = z__2.r * res.i + 
		z__2.i * res.r;
	parti = z__1.r;
/* Computing 2nd power */
	d__1 = partr;
/* Computing 2nd power */
	d__2 = parti;
	resmag = sqrt(d__1 * d__1 + d__2 * d__2);
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
	distr = (d__1 = r - 1., abs(d__1));
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
    integer i__1, i__2, i__3, i__4;
    doublereal d__1, d__2;
    doublecomplex z__1, z__2, z__3;

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

/*        mm=degree of polynomial */
/*        a=coefficient array, lowest to highest degree */
/*        kprint=1 for full printing */
/*        kerr=0 is normal return */
    /* Parameter adjustments */
    --rooti;
    --rootr;
    --a;

    /* Function Body */
    jay.r = 0., jay.i = 1.;
    mmp = *mm + 1;
    m = *mm;
    mp = mmp;
    i__1 = mp;
    for (i = 1; i <= i__1; ++i) {
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
/*      kpolm is number of attempted iterations before polishing is stoppe
d*/
    kpolm = 20;
/*       amax is the largest number we allow */
    amax = 4294967296.;
    amin = 1. / amax;
/*       rr1 and rr2 are radii within which we work for polishing */
    d__1 = 1. / m;
    rr1 = pow_dd(&amin, &d__1);
    d__1 = 1. / m;
    rr2 = pow_dd(&amax, &d__1);
/*     eps is the tolerance for convergence */
    sqteps = sqrt(*eps);
/*        main loop; m is current degree */
L10:
    if (m <= 0) {
	goto L200;
    }
/*        new z, a point on the unit circle */
    rkount = (doublereal) kount;
    d__1 = cos(rkount);
    d__2 = sin(rkount);
    z__2.r = d__2 * jay.r, z__2.i = d__2 * jay.i;
    z__1.r = d__1 + z__2.r, z__1.i = z__2.i;
    z.r = z__1.r, z.i = z__1.i;
    ktry = 0;
/*      r1 and r2 are boundaries of an expanding annulus within which we w
ork*/
    d__1 = 1. / m;
    r1 = pow_dd(&amin, &d__1);
    d__1 = 1. / m;
    r2 = pow_dd(&amax, &d__1);
/*        inside loop */
L20:
    partr = z.r;
    z__2.r = -jay.r, z__2.i = -jay.i;
    z__1.r = z__2.r * z.r - z__2.i * z.i, z__1.i = z__2.r * z.i + z__2.i * 
	    z.r;
    parti = z__1.r;
/* Computing 2nd power */
    d__1 = partr;
/* Computing 2nd power */
    d__2 = parti;
    size = sqrt(d__1 * d__1 + d__2 * d__2);
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
    i__1 = mp - 1;
    i__2 = mp;
    b[i__1].r = a[i__2], b[i__1].i = 0.;
    i__1 = m;
    for (j = 1; j <= i__1; ++j) {
	k = m - j + 1;
/* L30: */
	i__2 = k - 1;
	i__3 = k;
	z__2.r = z.r * b[i__3].r - z.i * b[i__3].i, z__2.i = z.r * b[i__3].i 
		+ z.i * b[i__3].r;
	i__4 = k;
	z__1.r = z__2.r + a[i__4], z__1.i = z__2.i;
	b[i__2].r = z__1.r, b[i__2].i = z__1.i;
    }
    p.r = b[0].r, p.i = b[0].i;
    partr = p.r;
    z__2.r = -jay.r, z__2.i = -jay.i;
    z__1.r = z__2.r * p.r - z__2.i * p.i, z__1.i = z__2.r * p.i + z__2.i * 
	    p.r;
    parti = z__1.r;
/* Computing 2nd power */
    d__1 = partr;
/* Computing 2nd power */
    d__2 = parti;
    if (sqrt(d__1 * d__1 + d__2 * d__2) > amax) {
	goto L300;
    }
/*        get value of derivative at z, synthetic division */
    i__2 = mp - 1;
    i__3 = mp - 1;
    c[i__2].r = b[i__3].r, c[i__2].i = b[i__3].i;
    mdec = m - 1;
    i__2 = mdec;
    for (j = 1; j <= i__2; ++j) {
	k = m - j + 1;
/* L60: */
	i__3 = k - 1;
	i__4 = k;
	z__2.r = z.r * c[i__4].r - z.i * c[i__4].i, z__2.i = z.r * c[i__4].i 
		+ z.i * c[i__4].r;
	i__1 = k - 1;
	z__1.r = z__2.r + b[i__1].r, z__1.i = z__2.i + b[i__1].i;
	c[i__3].r = z__1.r, c[i__3].i = z__1.i;
    }
    pp.r = c[1].r, pp.i = c[1].i;
    partr = pp.r;
    z__2.r = -jay.r, z__2.i = -jay.i;
    z__1.r = z__2.r * pp.r - z__2.i * pp.i, z__1.i = z__2.r * pp.i + z__2.i * 
	    pp.r;
    parti = z__1.r;
/* Computing 2nd power */
    d__1 = partr;
/* Computing 2nd power */
    d__2 = parti;
    if (sqrt(d__1 * d__1 + d__2 * d__2) < amin) {
	goto L300;
    }
/*        test for convergence */
    partr = p.r;
    z__2.r = -jay.r, z__2.i = -jay.i;
    z__1.r = z__2.r * p.r - z__2.i * p.i, z__1.i = z__2.r * p.i + z__2.i * 
	    p.r;
    parti = z__1.r;
/* Computing 2nd power */
    d__1 = partr;
/* Computing 2nd power */
    d__2 = parti;
    size = sqrt(d__1 * d__1 + d__2 * d__2);
    if (size > *eps) {
	goto L775;
    }
    nroot = *mm - m + 1;
    goto L500;
L775:
    z_div(&z__2, &p, &pp);
    z__1.r = z.r - z__2.r, z__1.i = z.i - z__2.i;
    z.r = z__1.r, z.i = z__1.i;
    goto L20;
/*        end of main loop */
/*        normal return */
L200:
    *kerr = 0;
    goto L600;
/*        new start */
L300:
    rkount = (doublereal) kount;
    d__1 = cos(rkount);
    d__2 = sin(rkount);
    z__2.r = d__2 * jay.r, z__2.i = d__2 * jay.i;
    z__1.r = d__1 + z__2.r, z__1.i = z__2.i;
    z.r = z__1.r, z.i = z__1.i;
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
    z__2.r = -jay.r, z__2.i = -jay.i;
    z__1.r = z__2.r * w.r - z__2.i * w.i, z__1.i = z__2.r * w.i + z__2.i * 
	    w.r;
    parti = z__1.r;
/* Computing 2nd power */
    d__1 = partr;
/* Computing 2nd power */
    d__2 = parti;
    size = sqrt(d__1 * d__1 + d__2 * d__2);
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
    i__3 = mmp - 1;
    i__4 = mmp - 1;
    bb[i__3].r = save[i__4], bb[i__3].i = 0.;
    i__3 = *mm;
    for (j = 1; j <= i__3; ++j) {
	k = *mm - j + 1;
/* L530: */
	i__4 = k - 1;
	i__1 = k;
	z__2.r = w.r * bb[i__1].r - w.i * bb[i__1].i, z__2.i = w.r * bb[i__1]
		.i + w.i * bb[i__1].r;
	i__2 = k - 1;
	z__1.r = z__2.r + save[i__2], z__1.i = z__2.i;
	bb[i__4].r = z__1.r, bb[i__4].i = z__1.i;
    }
    p.r = bb[0].r, p.i = bb[0].i;
    partr = p.r;
    z__2.r = -jay.r, z__2.i = -jay.i;
    z__1.r = z__2.r * p.r - z__2.i * p.i, z__1.i = z__2.r * p.i + z__2.i * 
	    p.r;
    parti = z__1.r;
/* Computing 2nd power */
    d__1 = partr;
/* Computing 2nd power */
    d__2 = parti;
    if (sqrt(d__1 * d__1 + d__2 * d__2) > amax) {
	goto L300;
    }
    i__4 = mmp - 1;
    i__1 = mmp - 1;
    cc[i__4].r = bb[i__1].r, cc[i__4].i = bb[i__1].i;
    mdec = *mm - 1;
    i__4 = mdec;
    for (j = 1; j <= i__4; ++j) {
	k = *mm - j + 1;
/* L560: */
	i__1 = k - 1;
	i__2 = k;
	z__2.r = w.r * cc[i__2].r - w.i * cc[i__2].i, z__2.i = w.r * cc[i__2]
		.i + w.i * cc[i__2].r;
	i__3 = k - 1;
	z__1.r = z__2.r + bb[i__3].r, z__1.i = z__2.i + bb[i__3].i;
	cc[i__1].r = z__1.r, cc[i__1].i = z__1.i;
    }
    pp.r = cc[1].r, pp.i = cc[1].i;
    partr = pp.r;
    z__2.r = -jay.r, z__2.i = -jay.i;
    z__1.r = z__2.r * pp.r - z__2.i * pp.i, z__1.i = z__2.r * pp.i + z__2.i * 
	    pp.r;
    parti = z__1.r;
/* Computing 2nd power */
    d__1 = partr;
/* Computing 2nd power */
    d__2 = parti;
    if (sqrt(d__1 * d__1 + d__2 * d__2) < amin) {
	goto L300;
    }
    partr = p.r;
    z__2.r = -jay.r, z__2.i = -jay.i;
    z__1.r = z__2.r * p.r - z__2.i * p.i, z__1.i = z__2.r * p.i + z__2.i * 
	    p.r;
    parti = z__1.r;
/* Computing 2nd power */
    d__1 = partr;
/* Computing 2nd power */
    d__2 = parti;
    size = sqrt(d__1 * d__1 + d__2 * d__2);
/*       test for convergence of polishing */
    if (size <= *eps) {
	goto L501;
    }
    z_div(&z__2, &p, &pp);
    z__1.r = w.r - z__2.r, z__1.i = w.i - z__2.i;
    w.r = z__1.r, w.i = z__1.i;
    goto L510;
/*        deflate */
L501:
    i__1 = mp - 1;
    i__2 = mp;
    b[i__1].r = a[i__2], b[i__1].i = 0.;
    i__1 = m;
    for (j = 1; j <= i__1; ++j) {
	k = m - j + 1;
/* L830: */
	i__2 = k - 1;
	i__3 = k;
	z__2.r = z.r * b[i__3].r - z.i * b[i__3].i, z__2.i = z.r * b[i__3].i 
		+ z.i * b[i__3].r;
	i__4 = k;
	z__1.r = z__2.r + a[i__4], z__1.i = z__2.i;
	b[i__2].r = z__1.r, b[i__2].i = z__1.i;
    }
    p.r = b[0].r, p.i = b[0].i;
    i__2 = m;
    rootr[i__2] = w.r;
    i__2 = m;
    z__2.r = -jay.r, z__2.i = -jay.i;
    z__1.r = z__2.r * w.r - z__2.i * w.i, z__1.i = z__2.r * w.i + z__2.i * 
	    w.r;
    rooti[i__2] = z__1.r;
    --m;
    --mp;
    z__2.r = -jay.r, z__2.i = -jay.i;
    z__1.r = z__2.r * w.r - z__2.i * w.i, z__1.i = z__2.r * w.i + z__2.i * 
	    w.r;
    parti = z__1.r;
    if (abs(parti) > sqteps) {
	goto L140;
    }
/*        real root */
    rooti[m + 1] = 0.;
    i__2 = mp;
    for (j = 1; j <= i__2; ++j) {
/* L100: */
	i__3 = j;
	i__4 = j;
	a[i__3] = b[i__4].r;
    }
    goto L10;
/*        complex root */
L140:
    partr = z.r;
    z__2.r = -jay.r, z__2.i = -jay.i;
    z__1.r = z__2.r * z.r - z__2.i * z.i, z__1.i = z__2.r * z.i + z__2.i * 
	    z.r;
    parti = z__1.r;
    z__2.r = parti * jay.r, z__2.i = parti * jay.i;
    z__1.r = partr - z__2.r, z__1.i = -z__2.i;
    z.r = z__1.r, z.i = z__1.i;
    i__3 = mp - 1;
    i__4 = mp;
    c[i__3].r = b[i__4].r, c[i__3].i = b[i__4].i;
    i__3 = m;
    for (j = 1; j <= i__3; ++j) {
	k = m - j + 1;
/* L110: */
	i__4 = k - 1;
	i__2 = k;
	z__2.r = z.r * c[i__2].r - z.i * c[i__2].i, z__2.i = z.r * c[i__2].i 
		+ z.i * c[i__2].r;
	i__1 = k;
	z__1.r = z__2.r + b[i__1].r, z__1.i = z__2.i + b[i__1].i;
	c[i__4].r = z__1.r, c[i__4].i = z__1.i;
    }
    i__4 = m;
    rootr[i__4] = w.r;
    i__4 = m;
    z__3.r = -jay.r, z__3.i = -jay.i;
    z__2.r = z__3.r * w.r - z__3.i * w.i, z__2.i = z__3.r * w.i + z__3.i * 
	    w.r;
    z__1.r = -z__2.r, z__1.i = -z__2.i;
    rooti[i__4] = z__1.r;
    --m;
    --mp;
    i__4 = mp;
    for (j = 1; j <= i__4; ++j) {
/* L130: */
	i__2 = j;
	i__1 = j;
	a[i__2] = c[i__1].r;
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

