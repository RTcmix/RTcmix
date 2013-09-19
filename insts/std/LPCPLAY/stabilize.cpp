// stabilize.C

#include "Complex.h"
//#include <stream.h>
#include "lp.h"

static int factor(double*, long*, double*, double*, long*, long*, double*);
static int dproot(long*, double*, double*, double*, long*, long*, double *);
static void correct(float*, long, float*);
static bool isStable(float*, long);

int
stabilize(float *frameIn, int npoles)
{
	int stable=0;
	float frameOut[MAXFRAME];
	for(int i=0; i<npoles; i++)
		frameOut[i] = -frameIn[npoles+3-i];
	if(!(stable = isStable(frameOut, npoles))) {
		correct(frameIn, npoles, frameOut);
		for (int n = 0; n < npoles; ++n)
			frameIn[n+4] = frameOut[n];
	}
	return stable;
}

// the remaining functions are UGLY because they were converted from Fortran

static Complex jay(0., 1.);
static Complex tmp;

inline double abs(double x) { return (x >= 0.) ? x : -x; }

static int
factor(double *b, long *k4, double *rootr, double *rooti,
	   long *kinsid, long *kprint, double *eps) 
{
    /* System generated locals */
    long i_1, i_2, i_3;

    /* Local variables */
    static double amax;
    static long kerr;
    static double dist, rmin, rmax;
    static long i, j, k;
    static double r;
    static Complex z, res;
    static double parti, distr, partr, r2, resmag, resmax;
    static long k4m;
    static double coe[MAXFRAME];

#ifdef LPC_DEBUG
	printf("in factor npoles = %d\n",*k4);
	for(i=0; i<36; i++) printf(" %g ",b[i]);
#endif
    /* Parameter adjustments */
    --b;
    --rootr;
    --rooti;
    /* Function Body */
/*       sets up problem, calls dproot, */
/*       and checks residual values at roots */
    i_1 = *k4;
    for (i = 1; i <= i_1; ++i) {
/* L550: */
		coe[i - 1] = b[i];
    }
    k4m = *k4 - 1;
    dproot(&k4m, coe, &rootr[1], &rooti[1], &kerr, kprint, eps);
    if (kerr > 0) {
    	return 0;
    }
    *kinsid = 0;
    resmax = 0.;
    rmax = 0.;
    rmin = 4294967296.;
    dist = 4294967296.;
    amax = 4294967296.;
    r2 = pow(amax, 1.0 / *k4);
    i_1 = k4m;
    for (j = 1; j <= i_1; ++j) {
		i_2 = j;
		i_3 = j;
		z = rootr[i_2] + (jay * rooti[i_3]);
	/* Computing 2nd power */
		r = hypot(rootr[j], rooti[j]);
	/*         skip residue calculation if root is too big */
		if (r > r2) {
			goto L713;
		}
		i_2 = *k4;
		res = b[i_2];
		i_2 = *k4;
		for (k = 2; k <= i_2; ++k) {
			i_3 = *k4 - k + 1;
			res = (res * z) + b[i_3];
		}
		partr = res.real();
		tmp = -jay * res;
		parti = tmp.real();
	/* Computing 2nd power */
		resmag = hypot(partr, parti);
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
		distr = abs(r - 1.);
		if (dist > distr) {
			dist = distr;
		}
    }
    return 1;
} /* factor */

static int
dproot(long *mm, double *a, double *rootr, double *rooti, long *kerr,
		long *kprint, double *eps) {
    /* System generated locals */
    long i_1, i_2, i_3, i_4;

    /* Local variables */
    static long mdec;
    static double amin, amax, save[MAXFRAME];
    static long kmax, kpol;
    static double temp, size;
    static long ktry;
    static double real1, real2;
    static Complex b[MAXFRAME], c[MAXFRAME];
    static Complex p, w, z;
    static double parti;
    static long kpolm;
    static double partr;
    static long kount, newst, nroot;
    static double r1, r2;
    static long ktrym;
    static Complex bb[MAXFRAME], cc[MAXFRAME];
    static long mp;
    static Complex pp;
    static double sqteps, rkount, rr1, rr2;
    static long mmp;

    int i, j, k, m;
	
    /* Parameter adjustments */
    --a;
    --rootr;
    --rooti;

    /* Function Body */
/*        mm=degree of polynomial */
/*        a=coefficient array, lowest to highest degree */
/*        kprint=1 for full printing */
/*        kerr=0 is normal return */
    mmp = *mm + 1;
    m = *mm;
    mp = mmp;
    i_1 = mp;
    for (i = 1; i <= i_1; ++i) {
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
/*      kpolm is number of attempted iterations before polishing is stopped */
    kpolm = 20;
/*       amax is the largest number we allow */
    amax = 4294967296.;
    amin = 1. / amax;
/*       rr1 and rr2 are radii within which we work for polishing */
    rr1 = pow(amin, 1. / m);
    rr2 = pow(amax, 1. / m);
/*     eps is the tolerance for convergence */
    sqteps = sqrt(*eps);
/*        main loop; m is current degree */
L10:
    if (m <= 0) {
	goto L200;
    }
/*        new z, a point on the unit circle */
    rkount = double(kount);
    z = (jay * sin(rkount)) + cos(rkount);
    ktry = 0;
/*      r1 and r2 are boundaries of an expanding annulus within which we 
work*/
    r1 = pow(amin, 1. / m);
    r2 = pow(amax, 1. / m);
/*        inside loop */
L20:
    partr = z.real();
    tmp = -jay * z;
    parti = tmp.real();
/* Computing 2nd power */
    size = hypot(partr, parti);
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
    b[i_1] = a[i_2];
    i_1 = m;
    for (j = 1; j <= i_1; ++j) {
		k = m - j + 1;
		i_2 = k - 1;
		i_3 = k;
		i_4 = k;
		b[i_2] = (z * b[i_3]) + a[i_4];
    }
    p = b[0];
    partr = p.real();
    tmp = -jay * p;
    parti = tmp.real();
/* Computing 2nd power */
    if (hypot(partr, parti) > amax) {
	goto L300;
    }
/*        get value of derivative at z, synthetic division */
    i_2 = mp - 1;
    i_3 = mp - 1;
    c[i_2] = b[i_3];
    mdec = m - 1;
    i_2 = mdec;
    for (j = 1; j <= i_2; ++j) {
		k = m - j + 1;
		i_3 = k - 1;
		i_4 = k;
		i_1 = k - 1;
		c[i_3] = (z * c[i_4]) + b[i_1];
    }
    pp = c[1];
    partr = pp.real();
    tmp = -jay * pp;
    parti = tmp.real();
/* Computing 2nd power */
    if (hypot(partr, parti) < amin) {
	goto L300;
    }
/*        test for convergence */
    partr = p.real();
    tmp = -jay * p;
    parti = tmp.real();
/* Computing 2nd power */
    size = hypot(partr, parti);
    if (size > *eps) {
	goto L775;
    }
    nroot = *mm - m + 1;
    goto L500;
L775:
    z = z - (p / pp);
    goto L20;
/*        end of main loop */
/*        normal return */
L200:
    *kerr = 0;
    goto L600;
/*        new start */
L300:
    rkount = double(kount);
    z = (jay * sin(rkount)) + cos(rkount);
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
    w = z;
    kpol = 0;
L510:
    partr = w.real();
    tmp = -jay * w;
    parti = tmp.real();
/* Computing 2nd power */
    size = hypot(partr, parti);
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
    bb[i_3] = save[i_4];
    i_3 = *mm;
    for (j = 1; j <= i_3; ++j) {
		k = *mm - j + 1;
		i_4 = k - 1;
		i_1 = k;
		i_2 = k - 1;
		bb[i_4] = (w * bb[i_1]) + save[i_2];
    }
    p = bb[0];
    partr = p.real();
    tmp = -jay * p;
    parti = tmp.real();
/* Computing 2nd power */
    if (hypot(partr, parti) > amax) {
	goto L300;
    }
    i_4 = mmp - 1;
    i_1 = mmp - 1;
    cc[i_4] = bb[i_1];
    mdec = *mm - 1;
    i_4 = mdec;
    for (j = 1; j <= i_4; ++j) {
		k = *mm - j + 1;
		i_1 = k - 1;
		i_2 = k;
		i_3 = k - 1;
		cc[i_1] = (w * cc[i_2]) + bb[i_3];
    }
    pp = cc[1];
    partr = pp.real();
    tmp = -jay * pp;
    parti = tmp.real();
/* Computing 2nd power */
    if (hypot(partr, parti) < amin) {
	goto L300;
    }
    partr = p.real();
    tmp = -jay * p;
    parti = tmp.real();
/* Computing 2nd power */
    size = hypot(partr, parti);
/*       test for convergence of polishing */
    if (size <= *eps) {
	goto L501;
    }
    w = w - (p / pp);
    goto L510;
/*        deflate */
L501:
    i_1 = mp - 1;
    i_2 = mp;
    b[i_1] = a[i_2];
    i_1 = m;
    for (j = 1; j <= i_1; ++j) {
		k = m - j + 1;
		i_2 = k - 1;
		i_3 = k;
		i_4 = k;
		b[i_2] = (z * b[i_3]) + a[i_4];
    }
    p = b[0];
    i_2 = m;
    rootr[i_2] = w.real();
    i_2 = m;
    tmp = -jay * w;
    rooti[i_2] = tmp.real();
    --m;
    --mp;
    tmp = -jay * w;
    parti = tmp.real();
    if (abs(parti) > sqteps) {
	goto L140;
    }
/*        real root */
    rooti[m + 1] = 0.;
    i_2 = mp;
    for (j = 1; j <= i_2; ++j) {
	i_3 = j;
	i_4 = j;
	a[i_3] = b[i_4].real();
    }
    goto L10;
/*        complex root */
L140:
    partr = z.real();
    tmp = -jay * z;
    parti = tmp.real();
    z = partr - (parti * jay);
    i_3 = mp - 1;
    i_4 = mp;
    c[i_3] = b[i_4];
    i_3 = m;
    for (j = 1; j <= i_3; ++j) {
		k = m - j + 1;
		i_4 = k - 1;
		i_2 = k;
		i_1 = k;
		c[i_4] = (z * c[i_2]) + b[i_1];
    }
    i_4 = m;
    rootr[i_4] = w.real();
    i_4 = m;
    tmp = -(-jay * w);
    rooti[i_4] = tmp.real();
    --m;
    --mp;
    i_4 = mp;
    for (j = 1; j <= i_4; ++j) {
		i_2 = j;
		i_1 = j;
		a[i_2] = c[i_1].real();
    }
    goto L10;
/*        report and return */
L600:
    real1 = double(kount);
    real2 = double((*mm));
    temp = real1 / real2;
    return 1;
} /* dproot */

static const Complex one = 1, zero = 0;

static void
correct(float *frame, long npoles, float *a) {
    /* System generated locals */
    long i,i_1, i_2, i_3, i_4, i_5;

    /* Local variables */
    static long nall;
    static long j, k;
    static double r[MAXFRAME];
    static Complex w[MAXFRAME];
    static long ndata;
    static double y[97], rooti[96];
    static long l1, k4;
    static double rootr[96];
    static long ii;
    static double th[MAXFRAME];
    static Complex ww;
    static long kinsid;
    static double zz;
    static long kprint, k4m;
    static double eps;

#ifdef LPC_DEBUG
	printf("\nin correct npoles = %d \n",npoles);
	for(i=0; i<36; i++) printf(" %g ",frame[i]);
#endif
    /* Parameter adjustments */
    --frame; 
    --a;
    /* Function Body */
    k4 = npoles + 1;
    k4m = k4 - 1;
    nall = k4 + 4;
    i_1 = k4m;
    for (ii = 1; ii <= i_1; ++ii) {
		y[ii - 1] = -frame[ii + 4];
    }
    y[k4 - 1] = 1.;
    eps = 1.0000000000000008e-8;
    factor(y, &k4, rootr, rooti, &kinsid, &kprint, &eps);
    i_1 = k4m;
    for (j = 1; j <= i_1; ++j) {
	/* Computing 2nd power */
		r[j - 1] = hypot(rootr[j - 1], rooti[j - 1]);
		th[j - 1] = atan2(rooti[j - 1], rootr[j - 1]);
		if (r[j - 1] >= 1.) {
			r[j - 1] = 1. / r[j - 1];
		}
    }
    i_1 = k4m;
    for (k = 1; k <= i_1; ++k) {
		i_2 = k - 1;
		w[i_2] = zero;
    }
    i_2 = k4 - 1;
    w[i_2] = one;
    i_2 = k4m;
    for (k = 1; k <= i_2; ++k) {
	/* 	ww=dcmplx(rootr(k),rooti(k)) */
		ww = polar(r[k - 1], th[k - 1]);
		l1 = k4 - k;
		i_1 = k4m;
		for (j = l1; j <= i_1; ++j) {
			i_3 = j - 1;
			i_4 = j;
			i_5 = j - 1;
			w[i_3] = w[i_4] - (ww * w[i_5]);
		}
		i_3 = k4 - 1;
		i_4 = k4 - 1;
		w[i_3] = -ww * w[i_4];
    }
    i_3 = k4;
    for (j = 2; j <= i_3; ++j) {
		i_4 = j - 1;
		zz = w[i_4].real();
		a[k4 + ndata + 1 - j] = -zz;
    }
}


static bool
isStable(float *frame, long npoles) {
    /* System generated locals */
    double r_1;
	static const int Size = 150;
	static const int SizeP1 = Size + 1;
    /* Local variables */
    static float a[Size * Size]	/* was [150][150] */;
    long i, m, mm;
    static float rk[249];
    /* Parameter adjustments */
    --frame;

    /* Function Body */
    a[(npoles + 1) * Size - Size] = 1.0;
    for (i = 1; i <= npoles; ++i) {
		a[i + 1 + (npoles + 1) * Size - SizeP1] = *(frame + i);
    }
    for (mm = 1; mm <= npoles; ++mm) {
		m = npoles - mm + 1;
		rk[m - 1] = a[m + 1 + (m + 1) * Size - SizeP1];
		if ((r_1 = rk[m - 1], abs(r_1)) < 1.) {
			goto L20;
		}
		return false;
	L20:
		for (i = 1; i <= m; ++i) {
	/* L25: */
	/* Computing 2nd power */
			r_1 = rk[m - 1];
			a[i + m * Size - SizeP1] = (a[i + (m + 1) * Size - SizeP1]
				- rk[m - 1] * a[m - i + 2 + (m + 1) * Size - SizeP1])
			/ (1.0 - r_1 * r_1);
		}
    }
    return true;
}
