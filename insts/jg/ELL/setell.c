/* setell.f -- translated by f2c (version of 25 March 1992  12:58:56).
   You must link the resulting object file with the libraries:
	-lF77 -lI77 -lm -lc   (in that order)
*/

#include <f2c.h>
#include "extramath.h"

/* Common Block Declarations */

struct {
    doublereal cn[30], cd[30];
    integer mn, md;
    doublereal const_;
} b_;

#define b_1 b_

struct {
    doublereal k, kprime, cosp0, w1, hpass;
} ellipt_;

#define ellipt_1 ellipt_

/* Table of constant values */

static integer c__200 = 200;
static doublereal c_b3 = 0.;
static doublereal c_b11 = 10.;

/* Subroutine */ int setell_(zsmpr, zf1, zf2, zf3, zripple, zatten, zretarr, 
	nsects)
real *zsmpr, *zf1, *zf2, *zf3, *zripple, *zatten, *zretarr;
integer *nsects;
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static doublereal smpr, xnyq;
    static integer i;
    static doublereal atten;
    extern /* Subroutine */ int fresp_(), reset_();
    static doublereal f1, f2, f3;
    static integer m2;
    extern /* Subroutine */ int ellips_();
    static doublereal ripple;
    static integer jjj;

    /* Parameter adjustments */
    --zretarr;

    /* Function Body */
    smpr = *zsmpr;
    f1 = *zf1;
    f2 = *zf2;
    f3 = *zf3;
    ripple = *zripple;
    atten = *zatten;
/*     print*, smpr, f1, f2, f3, ripple, atten, nsects */
    reset_();
    xnyq = smpr / 2.;
    ellips_(&f1, &f2, &f3, &ripple, &atten, &smpr);
    fresp_(&c__200, &smpr, &c_b3, &xnyq, &f1);
    m2 = b_1.mn / 2;
    *nsects = m2;
    jjj = 1;
    i__1 = b_1.mn;
    for (i = 1; i <= i__1; ++i) {
	zretarr[jjj] = b_1.cn[i - 1];
	zretarr[jjj + 1] = b_1.cd[i - 1];
	jjj += 2;
/* L1414: */
    }
    zretarr[jjj] = b_1.const_;
    return 0;
} /* setell_ */

/* Subroutine */ int reset_()
{
    static integer m;

    b_1.mn = 0;
    b_1.md = 0;
    for (m = 1; m <= 30; ++m) {
	b_1.cn[m - 1] = (float)0.;
	b_1.cd[m - 1] = (float)0.;
/* L100: */
    }
    return 0;
} /* reset_ */

/* Subroutine */ int ellips_(f1, f2, f3, ripple, atten, samr)
doublereal *f1, *f2, *f3, *ripple, *atten, *samr;
{
    /* System generated locals */
    integer i__1;
    doublereal d__1, d__2, d__3, d__4, d__5, d__6;

    /* Builtin functions */
    double tan(), cos(), sin(), sqrt(), pow_dd(), log();

    /* Local variables */
    static doublereal a;
    static integer i, n;
    static doublereal k1;
    static integer n2;
    static doublereal u0, w2, w3, k1prim, dd, de;
    extern /* Subroutine */ int stuff1_();
    static doublereal kk, pi, nn, tt, kk1;
    extern doublereal kay_();
    static doublereal kkp, eps, kk1p;

/*   designs an elliptic filter. all parameters real*8 . */
/*   f3=0 -> lowpass or highpass. f1=passband cutoff. f2=stopband cutoff. 
*/
/*   f1<f2 -> lowpass. */
/*   f3>0 -> bandpass. f1,f2 are limits of passband. f3 is limit of */
/*   either high or low stopband. we require f1<f2. */
/*   ripple=passband ripple in db. atten=stopband attenuation in db. */
/*   samr=sampling rate in hz. */
/*    after gold+rader; written by bilofsky, revised by steiglitz */
/*    pp.61-65 (elliptic filters), 72,76 (mappings */
/*    from s-plane to z-plane), 87 (approximation */
/*    for u0 and evaluation of elliptic functions). */
    pi = 3.14159265358979;
    ellipt_1.w1 = pi * 2. * *f1 / *samr;
    w2 = pi * 2. * *f2 / *samr;
    w3 = pi * 2. * *f3 / *samr;
    ellipt_1.hpass = 0.;
    ellipt_1.cosp0 = 0.;
    if (*f3 > 0.) {
	goto L1;
    }
    if (*f1 < *f2) {
	goto L2;
    }
/*  modify frequencies for high pass. */
    ellipt_1.w1 = pi - ellipt_1.w1;
    w2 = pi - w2;
    ellipt_1.hpass = 1.;
/*  compute analog frequencies for low/high pass */
L2:
    ellipt_1.w1 = tan(ellipt_1.w1 * .5);
    w2 = tan(w2 * .5);
    goto L3;
/*  compute analog frequencies for band pass. */
L1:
    ellipt_1.cosp0 = cos((ellipt_1.w1 + w2) / 2.) / cos((ellipt_1.w1 - w2) / 
	    2.);
    ellipt_1.w1 = (d__1 = (ellipt_1.cosp0 - cos(ellipt_1.w1)) / sin(
	    ellipt_1.w1), abs(d__1));
    de = w3 - w2;
    if (de < 0.) {
	de = ellipt_1.w1 - w3;
    }
    d__1 = ellipt_1.w1 - de;
    d__3 = w2 + de;
/* Computing MIN */
    d__5 = (d__2 = (ellipt_1.cosp0 - cos(d__1)) / sin(d__1), abs(d__2)), d__6 
	    = (d__4 = (ellipt_1.cosp0 - cos(d__3)) / sin(d__3), abs(d__4));
    w2 = min(d__5,d__6);
/*  compute params for poles,zeros in lambda plane */
L3:
    ellipt_1.k = ellipt_1.w1 / w2;
/* Computing 2nd power */
    d__1 = ellipt_1.k;
    ellipt_1.kprime = sqrt(1. - d__1 * d__1);
    d__1 = *ripple * .1;
    eps = sqrt(pow_dd(&c_b11, &d__1) - 1.);
    d__1 = *atten * .05;
    a = pow_dd(&c_b11, &d__1);
    k1 = eps / sqrt(a * a - 1.);
/* Computing 2nd power */
    d__1 = k1;
    k1prim = sqrt(1. - d__1 * d__1);
    kk = kay_(&ellipt_1.k);
    kk1 = kay_(&k1);
    kkp = kay_(&ellipt_1.kprime);
    kk1p = kay_(&k1prim);
    n = (integer) (kk1p * kk / (kk1 * kkp)) + 1;
    nn = (doublereal) n;
/* L5: */
    u0 = -kkp * log((sqrt(eps * eps + 1.) + 1.) / eps) / kk1p;
/*  now compute poles,zeros in lambda plane, */
/*    transform one by one to z plane. */
    dd = kk / nn;
    tt = kk - dd;
    dd += dd;
    n2 = (n + 1) / 2;
    i__1 = n2;
    for (i = 1; i <= i__1; ++i) {
	if (i << 1 > n) {
	    tt = 0.;
	}
	d__1 = -kkp;
	stuff1_(&d__1, &tt, "zero", 4L);
	stuff1_(&u0, &tt, "pole", 4L);
/* L4: */
	tt -= dd;
    }
    return 0;
} /* ellips_ */

/* Subroutine */ int stuff1_(q, r, whatsi, whatsi_len)
doublereal *q, *r;
char *whatsi;
ftnlen whatsi_len;
{
    /* System generated locals */
    doublereal d__1, d__2, d__3;
    doublecomplex z__1, z__2, z__3, z__4, z__5, z__6, z__7, z__8;

    /* Builtin functions */
    void z_div();
    double d_imag();
    void d_cnjg();
    integer s_cmp();

    /* Local variables */
    static doublereal cnqp, dnqp, snqp;
    static integer j;
    static doublecomplex s;
    extern /* Subroutine */ int djelf_();
    extern doublereal dreal_();
    static doublereal omega, x;
    static doublecomplex z;
    static doublereal sigma;
    extern /* Double Complex */ int cdsqrt_();
    static doublereal cnr, dnr, snr;

/*    transforms poles and zeros to z-plane; stuffs coeff. array */
    d__1 = ellipt_1.kprime * ellipt_1.kprime;
    djelf_(&snr, &cnr, &dnr, r, &d__1);
    d__1 = ellipt_1.k * ellipt_1.k;
    djelf_(&snqp, &cnqp, &dnqp, q, &d__1);
    omega = 1 - snqp * snqp * dnr * dnr;
    if (omega == 0.) {
	omega = 1e-30;
    }
    sigma = ellipt_1.w1 * snqp * cnqp * cnr * dnr / omega;
    omega = ellipt_1.w1 * snr * dnqp / omega;
    z__1.r = sigma, z__1.i = omega;
    s.r = z__1.r, s.i = z__1.i;
    j = 1;
    if (ellipt_1.cosp0 == 0.) {
	goto L1;
    }
    j = -1;
L4:
    d__1 = -ellipt_1.cosp0;
    d__2 = (doublereal) j;
    d__3 = ellipt_1.cosp0 * ellipt_1.cosp0;
    z__7.r = s.r * s.r - s.i * s.i, z__7.i = s.r * s.i + s.i * s.r;
    z__6.r = d__3 + z__7.r, z__6.i = z__7.i;
    z__5.r = z__6.r - 1., z__5.i = z__6.i;
    cdsqrt_(&z__4, &z__5);
    z__3.r = d__2 * z__4.r, z__3.i = d__2 * z__4.i;
    z__2.r = d__1 + z__3.r, z__2.i = z__3.i;
    z__8.r = s.r - 1., z__8.i = s.i;
    z_div(&z__1, &z__2, &z__8);
    z.r = z__1.r, z.i = z__1.i;
    goto L3;
L1:
    z__2.r = s.r + 1., z__2.i = s.i;
    z__3.r = 1. - s.r, z__3.i = -s.i;
    z_div(&z__1, &z__2, &z__3);
    z.r = z__1.r, z.i = z__1.i;
    if (ellipt_1.hpass != 0.) {
	z__1.r = -z.r, z__1.i = -z.i;
	z.r = z__1.r, z.i = z__1.i;
    }
L3:
    if ((d__1 = d_imag(&z), abs(d__1)) <= 1e-9) {
	goto L2;
    }
    if (d_imag(&z) < 0.) {
	d_cnjg(&z__1, &z);
	z.r = z__1.r, z.i = z__1.i;
    }
    if (s_cmp(whatsi, "pole", 4L, 4L) == 0) {
	goto L5;
    }
    ++b_1.mn;
    b_1.cn[b_1.mn - 1] = dreal_(&z) * -2.;
    ++b_1.mn;
/* Computing 2nd power */
    d__1 = dreal_(&z);
/* Computing 2nd power */
    d__2 = d_imag(&z);
    b_1.cn[b_1.mn - 1] = d__1 * d__1 + d__2 * d__2;
    goto L6;
L5:
    ++b_1.md;
    b_1.cd[b_1.md - 1] = dreal_(&z) * -2.;
    ++b_1.md;
/* Computing 2nd power */
    d__1 = dreal_(&z);
/* Computing 2nd power */
    d__2 = d_imag(&z);
    b_1.cd[b_1.md - 1] = d__1 * d__1 + d__2 * d__2;
L6:
/*    6 write(6,202)whatsi,z */
/* L202: */
    if (j > 0 || *r == 0.) {
	return 0;
    }
    j = 1;
    goto L4;
L2:
    x = dreal_(&z);
    if (s_cmp(whatsi, "pole", 4L, 4L) == 0) {
	goto L7;
    }
    ++b_1.mn;
    b_1.cn[b_1.mn - 1] = -x;
    ++b_1.mn;
    b_1.cn[b_1.mn - 1] = 0.;
    goto L8;
L7:
    ++b_1.md;
    b_1.cd[b_1.md - 1] = -x;
    ++b_1.md;
    b_1.cd[b_1.md - 1] = 0.;
L8:
/*    8 write(6,201)whatsi,x */
/* L201: */
    if (j > 0) {
	return 0;
    }
    j = 1;
    goto L4;
} /* stuff1_ */

/* Subroutine */ int fresp_(k, samr, f1, f2, f3)
integer *k;
doublereal *samr, *f1, *f2, *f3;
{
    /* System generated locals */
    integer i__1, i__2, i__3, i__4, i__5, i__6;
    doublereal d__1;
    doublecomplex z__1, z__2, z__3, z__4, z__5, z__6, z__7, z__8, z__9, z__10;


    /* Builtin functions */
    void z_div();
    double d_imag(), atan2(), d_lg10();

    /* Local variables */
    static doublereal freq;
    static integer i, j;
    extern doublereal cdabs_();
    static doublereal w, x;
    extern doublereal dreal_();
    static doublereal y, phase;
    extern /* Double Complex */ int cdexp_();
    static integer m2;
    static doublereal db, pi;
    static doublecomplex tf, zm, zm2;
    static doublereal amp;

/*    plots k pts. of freq. resp. from f1 to f2, norm. at f3 */
    pi = 3.14159265358979;
    m2 = b_1.mn / 2;
/*      write(8,200)m2,(cn(i),cd(i),i=1,mn) */
/* L200: */
    w = pi * *f3 / (*samr * .5);
    d__1 = w * -1.;
    z__2.r = 0., z__2.i = d__1;
    cdexp_(&z__1, &z__2);
    zm.r = z__1.r, zm.i = z__1.i;
    z__1.r = zm.r * zm.r - zm.i * zm.i, z__1.i = zm.r * zm.i + zm.i * zm.r;
    zm2.r = z__1.r, zm2.i = z__1.i;
    tf.r = 1., tf.i = 0.;
    i__1 = b_1.mn;
    for (i = 1; i <= i__1; i += 2) {
/* L1: */
	i__2 = i - 1;
	z__5.r = b_1.cn[i__2] * zm.r, z__5.i = b_1.cn[i__2] * zm.i;
	z__4.r = z__5.r + 1., z__4.i = z__5.i;
	i__3 = i;
	z__6.r = b_1.cn[i__3] * zm2.r, z__6.i = b_1.cn[i__3] * zm2.i;
	z__3.r = z__4.r + z__6.r, z__3.i = z__4.i + z__6.i;
	z__2.r = tf.r * z__3.r - tf.i * z__3.i, z__2.i = tf.r * z__3.i + tf.i 
		* z__3.r;
	i__4 = i - 1;
	z__9.r = b_1.cd[i__4] * zm.r, z__9.i = b_1.cd[i__4] * zm.i;
	z__8.r = z__9.r + 1., z__8.i = z__9.i;
	i__5 = i;
	z__10.r = b_1.cd[i__5] * zm2.r, z__10.i = b_1.cd[i__5] * zm2.i;
	z__7.r = z__8.r + z__10.r, z__7.i = z__8.i + z__10.i;
	z_div(&z__1, &z__2, &z__7);
	tf.r = z__1.r, tf.i = z__1.i;
    }
    b_1.const_ = 1. / cdabs_(&tf);
/*      write(8,201)const */
/* L201: */
/*      write(8,205) */
/* L205: */
    i__2 = *k;
    for (j = 1; j <= i__2; ++j) {
	freq = *f1 + (*f2 - *f1) * (doublereal) (j - 1) / (doublereal) (*k - 
		1);
	w = pi * freq / (*samr * .5);
	d__1 = w * -1.;
	z__2.r = 0., z__2.i = d__1;
	cdexp_(&z__1, &z__2);
	zm.r = z__1.r, zm.i = z__1.i;
	z__1.r = zm.r * zm.r - zm.i * zm.i, z__1.i = zm.r * zm.i + zm.i * 
		zm.r;
	zm2.r = z__1.r, zm2.i = z__1.i;
	z__1.r = b_1.const_, z__1.i = 0.;
	tf.r = z__1.r, tf.i = z__1.i;
	i__3 = b_1.mn;
	for (i = 1; i <= i__3; i += 2) {
/* L2: */
	    i__4 = i - 1;
	    z__5.r = b_1.cn[i__4] * zm.r, z__5.i = b_1.cn[i__4] * zm.i;
	    z__4.r = z__5.r + 1., z__4.i = z__5.i;
	    i__5 = i;
	    z__6.r = b_1.cn[i__5] * zm2.r, z__6.i = b_1.cn[i__5] * zm2.i;
	    z__3.r = z__4.r + z__6.r, z__3.i = z__4.i + z__6.i;
	    z__2.r = tf.r * z__3.r - tf.i * z__3.i, z__2.i = tf.r * z__3.i + 
		    tf.i * z__3.r;
	    i__1 = i - 1;
	    z__9.r = b_1.cd[i__1] * zm.r, z__9.i = b_1.cd[i__1] * zm.i;
	    z__8.r = z__9.r + 1., z__8.i = z__9.i;
	    i__6 = i;
	    z__10.r = b_1.cd[i__6] * zm2.r, z__10.i = b_1.cd[i__6] * zm2.i;
	    z__7.r = z__8.r + z__10.r, z__7.i = z__8.i + z__10.i;
	    z_div(&z__1, &z__2, &z__7);
	    tf.r = z__1.r, tf.i = z__1.i;
	}
	amp = cdabs_(&tf);
	if (amp <= 1e-20) {
	    amp = 1e-20;
	}
	x = dreal_(&tf);
	y = d_imag(&tf);
	phase = 0.;
	if (x == 0. && y == 0.) {
	    goto L4;
	}
	phase = 180. / pi * atan2(y, x);
L4:
	d__1 = max(amp,1e-40);
	db = d_lg10(&d__1) * 20.;
/* L3: */
    }
/*    3 write(8,202)freq,phase,amp,db */
/* L202: */
    return 0;
} /* fresp_ */

doublereal kay_(k)
doublereal *k;
{
    /* Initialized data */

    static doublereal a[5] = { 1.38629436112,.09666344259,.03590092383,
	    .03742563713,.01451196212 };
    static doublereal b[5] = { .5,.12498593597,.06880248576,.03328355346,
	    .00441787012 };

    /* System generated locals */
    doublereal ret_val;

    /* Builtin functions */
    double log();

    /* Local variables */
    static doublereal peta;
    static integer i;
    static doublereal kk, eta;

/*    computes kay(k)=inverse sn(1) */
/*    hastings, approx. for dig. comp., p. 172 */
    ret_val = a[0];
    kk = b[0];
    eta = 1. - *k * *k;
    peta = eta;
    for (i = 2; i <= 5; ++i) {
	ret_val += a[i - 1] * peta;
	kk += b[i - 1] * peta;
/* L1: */
	peta *= eta;
    }
    ret_val -= kk * log(eta);
    return ret_val;
} /* kay_ */

/* Subroutine */ int djelf_(sn, cn, dn, x, sck)
doublereal *sn, *cn, *dn, *x, *sck;
{
    /* System generated locals */
    integer i__1;
    doublereal d__1;

    /* Builtin functions */
    double exp(), sqrt(), sin(), cos();

    /* Local variables */
    static doublereal a, b, c, d;
    static integer i, k, l;
    static doublereal y, cm, geo[12], ari[12];

/*     ssp program: finds jacobian elliptic functions sn,cn,dn. */
    cm = *sck;
    y = *x;
    if (*sck < 0.) {
	goto L3;
    } else if (*sck == 0) {
	goto L1;
    } else {
	goto L4;
    }
L1:
    d = exp(*x);
    a = 1. / d;
    b = a + d;
    *cn = 2. / b;
    *dn = *cn;
    a = (d - a) / 2.;
    *sn = a * *cn;
L2:
    return 0;
L3:
    d = 1. - *sck;
    cm = -(*sck) / d;
    d = sqrt(d);
    y = d * *x;
L4:
    a = 1.;
    *dn = 1.;
    for (i = 1; i <= 12; ++i) {
	l = i;
	ari[i - 1] = a;
	cm = sqrt(cm);
	geo[i - 1] = cm;
	c = (a + cm) * .5;
	if ((d__1 = a - cm, abs(d__1)) - a * 1e-9 <= 0.) {
	    goto L7;
	} else {
	    goto L5;
	}
L5:
	cm = a * cm;
/* L6: */
	a = c;
    }
L7:
    y = c * y;
    *sn = sin(y);
    *cn = cos(y);
    if (*sn != 0.) {
	goto L8;
    } else {
	goto L13;
    }
L8:
    a = *cn / *sn;
    c = a * c;
    i__1 = l;
    for (i = 1; i <= i__1; ++i) {
	k = l - i + 1;
	b = ari[k - 1];
	a = c * a;
	c = *dn * c;
	*dn = (geo[k - 1] + a) / (b + a);
/* L9: */
	a = c / b;
    }
    a = 1. / sqrt(c * c + 1.);
    if (*sn >= 0.) {
	goto L11;
    } else {
	goto L10;
    }
L10:
    *sn = -a;
    goto L12;
L11:
    *sn = a;
L12:
    *cn = c * *sn;
L13:
    if (*sck >= 0.) {
	goto L2;
    } else {
	goto L14;
    }
L14:
    a = *dn;
    *dn = *cn;
    *cn = a;
    *sn /= d;
    return 0;
} /* djelf_ */

