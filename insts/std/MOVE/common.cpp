// common.C -- common implementation for PLACE and MOVE

#include "common.h"
#include <math.h>
#include <stdio.h>

double SINARRAY[1025], COSARRAY[1025], ATANARRAY[1025];

/* ----------------------------------------------------------------- wrap --- */
/* Converts negative or large polar angles (in rads) into positive */
double
wrap(double x)
{
   register double val = x;
   while (val < 0.0)
      val += PI2;
   while (val >= PI2)
      val -= PI2;
   return (val);
}

/* ---------------------------------------------------------------- cycle --- */
/* Determines the sampling increment for a table of any size based on
   the frequency desired.
*/
float
cycle(float SR, double freq, int funsize)
{
   return (freq * funsize / SR);
}

/* ----------------------------------------------------------------- scale --- */
/* scale attenuates signal */
void
scale(double *Sig, int len, double amp)
{
	for (int i = 0; i < len; ++i)
		Sig[i] *= amp;
}

void
btone(double *Sig, int len, double data[3])
{
	const double d0 = data[0];
	const double d1 = data[1];
	double past = data[2];
	const int lenOver4 = len >> 2;
	const int remainder = len - (lenOver4 << 2);
	for (int i = 0; i < lenOver4; ++i) {
		past = d0 * Sig[0] + d1 * past;
		Sig[0] = past;
		past = d0 * Sig[1] + d1 * past;
		Sig[1] = past;
		past = d0 * Sig[2] + d1 * past;
		Sig[2] = past;
		past = d0 * Sig[3] + d1 * past;
		Sig[3] = past;
		Sig += 4;
	}
	for (int i = 0; i < remainder; ++i) {
		past = d0 * Sig[i] + d1 * past;
		Sig[i] = past;
	}
	data[2] = past;
}

#if defined(i386)
float airOffset =  1.0e-35;
#endif

/* ------------------------------------------------------------------ air --- */
/* air is the tone filter that simulates air absorption */
void
air(double *Sig, int len, double airdata[3])
{
	btone(Sig, len, airdata);
	for (int i = 0; i < len; ++i) {
#if defined(i386)
		Sig[i] += airOffset;	// adding this small offset avoids FP underflow
		airOffset = -airOffset;
#endif
	}
}


/* ----------------------------------------------------------------- wall --- */
/* wall is the tone filter that simulates absorption of high
   frequencies by the walls of the room. These are set up in the separate
   setup routine, space.
*/
#if defined(i386)
float wallOffset = -1.0e-35;
#endif

void
wall(double *Sig, int len, double Walldata[3])
{
	btone(Sig, len, &Walldata[0]);
	for (int i = 0; i < len; ++i) {
#if defined(i386)
		Sig[i] += wallOffset;	// adding this small offset avoids FP underflow
		wallOffset = -wallOffset;
#endif
	}
}

// other buffer routines

template <typename T>
static void copyBufTo(T *to, double *from, int len)
{
	const int len4 = len >> 2;
	int i;
	for (i = 0; i < len4; i++) {
	    to[0] = from[0];
	    to[1] = from[1];
	    to[2] = from[2];
	    to[3] = from[3];
		to += 4;
		from += 4;
	}
	const int extra = len - (len4<<2);
	for (i = 0; i < extra; i++)
	    *to++ = *from++;
}

void copyBuf(double *to, double *from, int len) { copyBufTo<double>(to, from, len); }

void copyBufToOut(float *to, double *from, int toChannels, int len)
{
	const int len4 = len >> 2;
	int i;
	const int second = toChannels;
	const int third = toChannels * 2;
	const int fourth = toChannels * 3;
	const int stride = toChannels * 4;
	for (i = 0; i < len4; i++) {
	    to[0] = from[0];
	    to[second] = from[1];
	    to[third] = from[2];
	    to[fourth] = from[3];
		to += stride;
		from += 4;
	}
	const int extra = len - (len4<<2);
	for (int n = 0; n < extra; ++n) {
	    *to += *from++;
		to += toChannels;
	}
}

void
copyScaleBuf(double *to, double *from, int len, double gain)
{
    if (gain == 1.0) {
        copyBuf(to, from, len);
        return;
    }
	const int len4 = len >> 2;
	int i;
	for (i = 0; i < len4; i++) {
	    to[0] = from[0] * gain;
	    to[1] = from[1] * gain;
	    to[2] = from[2] * gain;
	    to[3] = from[3] * gain;
		to += 4;
		from += 4;
	}
	const int extra = len - (len4<<2);
	for (i = 0; i < extra; i++)
	    *to++ = *from++ * gain;
}

template <typename T>
static void addBufTo(T *to, double *from, int len)
{
	const int len4 = len >> 2;
	int i;
	for (i = 0; i < len4; i++) {
	    to[0] += from[0];
	    to[1] += from[1];
	    to[2] += from[2];
	    to[3] += from[3];
		to += 4;
		from += 4;
	}
	const int extra = len - (len4<<2);
	for (i = 0; i < extra; i++)
	    *to++ += *from++;
}

void addBuf(double *to, double *from, int len) { addBufTo(to, from, len); }

template <typename T>
static void addScaleBufTo(T *to, double *from, int len, double gain)
{
    if (gain == 0.0) return;
    else if (gain == 1.0) { addBufTo(to, from, len); return; }
	const int len4 = len >> 2;
	int i;
	for (i = 0; i < len4; i++) {
	    to[0] += from[0] * gain;
	    to[1] += from[1] * gain;
	    to[2] += from[2] * gain;
	    to[3] += from[3] * gain;
		to += 4;
		from += 4;
	}
	const int extra = len - (len4<<2);
	for (i = 0; i < extra; i++)
	    *to++ += *from++ * gain;
}

void addScaleBuf(double *to, double *from, int len, double gain) { addScaleBufTo(to, from, len, gain); }

void addScaleBufToOut(float *to, double *from, int len, int toChannels, double gain) {
    if (gain == 0.0) return;
	const int len4 = len >> 2;
	int i;
	const int second = toChannels;
	const int third = toChannels * 2;
	const int fourth = toChannels * 3;
	const int stride = toChannels * 4;
	for (i = 0; i < len4; i++) {
	    to[0] += from[0] * gain;
	    to[second] += from[1] * gain;
	    to[third] += from[2] * gain;
	    to[fourth] += from[3] * gain;
		to += stride;
		from += 4;
	}
	const int extra = len - (len4<<2);
	for (i = 0; i < extra; ++i) {
	    *to += *from++ * gain;
		to += toChannels;
	}
}

#if defined(i386)
/* -------------------------------------------------- * check_denormals --- */
/* check_denormals assures that no values in the buffer underflow the FPU */
void
check_denormals(double *Sig, int len)
{
	for (int i = 0; i < len; ++i)
		if ((Sig[i] > 0.0 && Sig[i] < 1.0e-30) || (Sig[i] < 0.0 && Sig[i] > -1.0e-30))
			Sig[i] = 0.0;
}
#endif

/* ------------------------------------------------------- setup_trigfuns --- */
/* Loads global arrays with sine and cosine functions for macros.
*/
void
setup_trigfuns()
{
   static int trigfuns_inited = 0;

   if (trigfuns_inited)
      return;

   for (int i = 0; i < 1024; i++) {
      SINARRAY[i] = sin((double) i * PI2 / 1024.0);
      COSARRAY[i] = cos((double) i * PI2 / 1024.0);
      ATANARRAY[i] = atan((double) i * PI / 1024.0 - (PI / 2.0));
   }
   SINARRAY[1024] = 0.0;
   COSARRAY[1024] = 1.0;
   ATANARRAY[1024] = 1.0;

   trigfuns_inited = 1;
}

/* -------------------------------------------------------------------------- */
/* The following functions from the original space.c. */
/* -------------------------------------------------------------------------- */


/* ------------------------------------------------------------ MFP_samps --- */
/* Computes the mean free path of reflections in the given room,
   and returns a value equal to the no. of samps delay for that path.
   Also prints stats about room and average delay to stdout.
*/

long
MFP_samps(float SR, double dim[])
{
   double volume, length, width, height, area, MFP, mean_delay;

   /* volume & surface area */
   length = dim[0] - dim[2];
   width = dim[1] - dim[3];
   height = dim[4];
   volume = length * width * height;
   area = 2.0 * (length * width + length * height + width * height);

   /* compute MFP */
   MFP = 4.0 * volume / area;
   mean_delay = 2.0 * MFP / MACH1;

   rtcmix_advise(NULL, "This room is %.1f ft. long,", length);
   rtcmix_advise(NULL, " %.1f ft. wide, & %.1f ft. high.\n", width, height);
   rtcmix_advise(NULL, "The average delay is %.2f ms.\n", mean_delay * 1000.0);

   return (long)(mean_delay * SR + 0.5);
}


/* ---------------------------------------------------------- close_prime --- */
/* Returns the closest prime number to a given input number <x>,
   up through the <n>th prime in array <p>.
   For use in determining delay lengths.
*/
int
close_prime(int x, int n, int p[])
{
   for (int i = 0; i < n; i++) {
      if (p[i] >= x)
         return (p[i]);
   }
   return p[n - 1];
}

/* ------------------------------------------------------------- binaural --- */
/* binaural computes the stereo distance/angle pairs for roomtrig */
void
binaural(double R,
         double T,
         double X,
         double Y,
         double H,             /* distance, angle, coordinates, */
         double *rho,
         double *theta)        /* "ear" dist., output arrays    */
{
   double h, Ys;

   /* store angle pair if head and angle are nonzero */

   if (T != 0.0) {
      theta[0] = wrap(-(T));
      theta[1] = wrap(T);
   }
   else
      theta[0] = theta[1] = T;

   /* calculate distance pair */

   h = H / 2.;
   Ys = Y * Y;

   /* distance to left ear */

   if (H > 0.8 || H == 0.0) {                          /* mike mode */
      rho[0] = sqrt(Ys + ((X + h) * (X + h)));
      rho[1] = sqrt(Ys + ((X - h) * (X - h)));
   }
   else {                                              /* binaural mode */
      double rightTheta = theta[1];                    /* positive RHS angle */
      if (rightTheta <= M_PI_2) {                      /* 1st quadrant */
         double angle = rightTheta;
         rho[1] = sqrt(Ys + ((X - h) * (X - h)));
         rho[0] = rho[1] + (h * (angle + sin(angle)));
      }
      else if (rightTheta < PI) {                      /* 2nd quadrant */
         double angle = PI - rightTheta;
         rho[1] = sqrt(Ys + ((X - h) * (X - h)));
         rho[0] = rho[1] + (h * (angle + sin(angle)));
      }
      else if (rightTheta < 3.0 * PI / 2.0) {          /* 3rd quadrant */
         double angle = rightTheta - PI;
         rho[0] = sqrt(Ys + ((X + h) * (X + h)));
         rho[1] = rho[0] + (h * (angle + sin(angle)));
      }
      else {                                           /* 4th quadrant */
         double angle = (2.0 * PI) - rightTheta;
         rho[0] = sqrt(Ys + ((X + h) * (X + h)));
         rho[1] = rho[0] + (h * (angle + sin(angle)));
      }
   }
}


/* ------------------------------------------------------------------ fir --- */
/* fir is a f.i.r. filter, set up by setfir, which filters each image
   sample pair to simulate binaural spectral cues according to angle
*/
void
fir(double *sig, long counter, int nterms, double *coeffs, double *firtap, int len)
{
	/* tap delay must be one longer than # of coeffs */

	const int flen = nterms + 1;
	int intap = counter % flen;    /* input location */

	for (int n = 0; n < len; n++, intap++) {
        if (intap >= flen)
            intap = 0;
		int outtap = intap - 1;
		int i, c = 0;

		firtap[intap] = sig[n];      /* load input sample into delay */

		/* for each of the delayed output samps, mult. by coeff and sum */

		double out = 0.0;

		/* first loop:  while outtap is >= 0 */
		const int loop1 = outtap + 1;
		for (i = 0; i < loop1; ++i, outtap--, c++) {
			out += firtap[outtap] * coeffs[c];
		}

		/* second loop: while outtap is positive after being wrapped */
		outtap += flen;
		const int loop2 = flen - loop1 - 1;
		for (i = 0; i < loop2; ++i, outtap--, c++) {
			out += firtap[outtap] * coeffs[c];
		}

		sig[n] = out;
	}
}


/* common data structures for handling FIR coefficients, etc. */

int g_Nterms[13] = {33, 25, 25, 25, 25, 15, 15, 15, 15, 15, 15, 15, 15};
int g_Group_delay[13] = {17, 13, 13, 13, 13, 8, 8, 8, 8, 8, 8, 8, 8};

const double FIRDATA[NANGLES][MAXTERMS] = {
{0.000000e+00, 0.000000e+00, 0.000000e+00, 0.000000e+00, 0.000000e+00, 
0.000000e+00, 0.000000e+00, 0.000000e+00, 0.000000e+00, 0.000000e+00, 
0.000000e+00, 0.000000e+00, 0.000000e+00, 0.000000e+00, 0.000000e+00, 
0.000000e+00, 1.000000e+00, 0.000000e+00, 0.000000e+00, 0.000000e+00, 
0.000000e+00, 0.000000e+00, 0.000000e+00, 0.000000e+00, 0.000000e+00, 
0.000000e+00, 0.000000e+00, 0.000000e+00, 0.000000e+00, 0.000000e+00, 
0.000000e+00, 0.000000e+00, 0.000000e+00}, 
{6.580588e-05, 7.266523e-05, -1.743364e-04, 2.204910e-04, 9.379746e-04, 
-1.018764e-03, -3.805183e-03, 5.983716e-04, 6.178302e-03, -2.877664e-03, 
-1.192994e-02, 2.440105e-04, -2.119257e-03, -4.097926e-02, -3.122661e-02, 
7.523096e-02, 1.147071e+00, 7.523096e-02, -3.122661e-02, -4.097926e-02, 
-2.119257e-03, 2.440105e-04, -1.192994e-02, -2.877664e-03, 6.178302e-03, 
5.983716e-04, -3.805182e-03, -1.018764e-03, 9.379748e-04, 2.204910e-04, 
-1.743364e-04, 7.266522e-05, 6.580588e-05}, 
{9.432345e-05, 1.740527e-04, -9.954048e-05, 3.547275e-04, 5.748939e-04, 
-3.570732e-03, -7.572739e-03, 7.622677e-04, 1.056869e-02, -1.531422e-04, 
-9.376652e-03, 2.582985e-03, -2.260461e-02, -9.145695e-02, -5.010163e-02, 
1.561503e-01, 1.286613e+00, 1.561503e-01, -5.010163e-02, -9.145694e-02, 
-2.260461e-02, 2.582985e-03, -9.376653e-03, -1.531422e-04, 1.056869e-02, 
7.622676e-04, -7.572737e-03, -3.570733e-03, 5.748940e-04, 3.547275e-04, 
-9.954048e-05, 1.740527e-04, 9.432345e-05}, 
{2.656524e-04, 4.424917e-04, 3.597317e-04, 1.034547e-03, 7.678652e-05, 
-7.935075e-03, -1.501109e-02, -3.468302e-03, 1.465996e-02, 1.316964e-02, 
1.009017e-02, 1.043696e-02, -5.197992e-02, -1.456310e-01, -6.456471e-02, 
2.274294e-01, 1.403165e+00, 2.274295e-01, -6.456471e-02, -1.456310e-01, 
-5.197992e-02, 1.043696e-02, 1.009017e-02, 1.316964e-02, 1.465996e-02, 
-3.468301e-03, -1.501109e-02, -7.935077e-03, 7.678653e-05, 1.034547e-03, 
3.597317e-04, 4.424916e-04, 2.656524e-04}, 
{2.457904e-04, 6.910631e-04, 7.887032e-04, 5.219169e-04, -2.206971e-03, 
-1.107647e-02, -2.099808e-02, -1.388319e-02, 1.346854e-02, 3.565251e-02, 
4.218335e-02, 2.725454e-02, -6.244072e-02, -1.845121e-01, -9.413760e-02, 
2.677864e-01, 1.490965e+00, 2.677864e-01, -9.413760e-02, -1.845121e-01, 
-6.244072e-02, 2.725454e-02, 4.218335e-02, 3.565251e-02, 1.346854e-02, 
-1.388318e-02, -2.099807e-02, -1.107647e-02, -2.206971e-03, 5.219169e-04, 
7.887031e-04, 6.910631e-04, 2.457904e-04}, 
{-7.048959e-04, 5.364980e-04, 2.034776e-03, 4.219607e-04, -4.453945e-03, 
-1.054524e-02, -2.533106e-02, -3.178074e-02, 1.400931e-02, 7.268960e-02, 
6.889405e-02, 4.035178e-02, -2.914208e-02, -1.985392e-01, -1.750240e-01, 
2.686839e-01, 1.582037e+00, 2.686839e-01, -1.750240e-01, -1.985392e-01, 
-2.914208e-02, 4.035178e-02, 6.889406e-02, 7.268960e-02, 1.400931e-02, 
-3.178074e-02, -2.533106e-02, -1.054524e-02, -4.453946e-03, 4.219607e-04, 
2.034776e-03, 5.364979e-04, -7.048959e-04}, 
{-2.008061e-03, 5.046264e-04, 3.396749e-03, -6.072934e-04, -5.707648e-03, 
-3.884167e-03, -2.622961e-02, -5.275417e-02, 1.802752e-02, 1.113747e-01, 
7.246644e-02, 4.150785e-02, 4.590280e-02, -1.760638e-01, -2.834539e-01, 
2.279393e-01, 1.648788e+00, 2.279393e-01, -2.834539e-01, -1.760638e-01, 
4.590280e-02, 4.150786e-02, 7.246645e-02, 1.113747e-01, 1.802751e-02, 
-5.275417e-02, -2.622960e-02, -3.884168e-03, -5.707649e-03, -6.072935e-04, 
3.396748e-03, 5.046263e-04, -2.008061e-03}, 
{-3.183930e-03, -2.306088e-04, 3.378053e-03, -9.026811e-04, -5.088841e-03, 
1.136346e-03, -2.105627e-02, -5.316117e-02, 1.979965e-02, 1.129706e-01, 
6.060518e-02, 4.849430e-02, 1.070919e-01, -1.229803e-01, -3.109830e-01, 
1.549340e-01, 1.575778e+00, 1.549341e-01, -3.109830e-01, -1.229803e-01, 
1.070920e-01, 4.849431e-02, 6.060519e-02, 1.129706e-01, 1.979965e-02, 
-5.316117e-02, -2.105627e-02, 1.136346e-03, -5.088842e-03, -9.026812e-04, 
3.378052e-03, -2.306088e-04, -3.183930e-03}, 
{-3.234652e-03, -1.200837e-03, 1.564698e-03, -8.738059e-04, -2.720420e-03, 
2.174022e-03, -1.716208e-02, -4.319096e-02, 1.538596e-02, 8.675188e-02, 
4.636974e-02, 5.914684e-02, 1.334001e-01, -6.066787e-02, -2.544940e-01, 
8.798884e-02, 1.418553e+00, 8.798885e-02, -2.544940e-01, -6.066787e-02, 
1.334001e-01, 5.914685e-02, 4.636975e-02, 8.675189e-02, 1.538596e-02, 
-4.319096e-02, -1.716208e-02, 2.174023e-03, -2.720421e-03, -8.738060e-04, 
1.564698e-03, -1.200837e-03, -3.234652e-03}, 
{-3.013577e-03, -1.935232e-03, 1.238112e-04, -8.957994e-04, -2.329834e-03, 
-1.326316e-06, -1.394866e-02, -3.207616e-02, 9.422473e-03, 6.037341e-02, 
3.297343e-02, 5.132664e-02, 1.278031e-01, 6.206067e-03, -1.567292e-01, 
3.122635e-02, 1.236832e+00, 3.122635e-02, -1.567292e-01, 6.206066e-03, 
1.278031e-01, 5.132665e-02, 3.297344e-02, 6.037341e-02, 9.422473e-03, 
-3.207616e-02, -1.394866e-02, -1.326310e-06, -2.329835e-03, -8.957995e-04, 
1.238112e-04, -1.935232e-03, -3.013577e-03}, 
{-2.365201e-03, -2.005247e-03, -7.713528e-04, -1.110750e-03, -2.441340e-03, 
-1.837691e-03, -1.005864e-02, -2.218447e-02, 5.780008e-04, 3.491741e-02, 
2.454314e-02, 3.673013e-02, 9.483628e-02, 4.873331e-02, -5.617650e-02, 
-5.168777e-03, 1.077633e+00, -5.168777e-03, -5.617650e-02, 4.873331e-02, 
9.483629e-02, 3.673014e-02, 2.454314e-02, 3.491741e-02, 5.780008e-04, 
-2.218447e-02, -1.005864e-02, -1.837691e-03, -2.441341e-03, -1.110750e-03, 
-7.713528e-04, -2.005247e-03, -2.365201e-03}, 
{-1.608268e-03, -1.810658e-03, -1.259604e-03, -7.563394e-04, -1.447662e-03, 
-2.393833e-03, -5.660108e-03, -1.101821e-02, -4.133298e-03, 1.506453e-02, 
2.220706e-02, 2.629418e-02, 5.219177e-02, 6.242390e-02, 1.942261e-02, 
-3.566079e-02, 9.434796e-01, -3.566079e-02, 1.942261e-02, 6.242389e-02, 
5.219177e-02, 2.629418e-02, 2.220706e-02, 1.506453e-02, -4.133297e-03, 
-1.101821e-02, -5.660107e-03, -2.393834e-03, -1.447663e-03, -7.563394e-04, 
-1.259603e-03, -1.810658e-03, -1.608268e-03}, 
{-8.234773e-04, -1.067473e-03, -1.380687e-03, -9.431105e-04, -2.002750e-04, 
-1.434359e-03, -2.905945e-03, -2.557954e-03, -2.240618e-03, 6.935375e-03, 
2.384733e-02, 1.976698e-02, 1.726856e-02, 6.332111e-02, 6.475365e-02, 
-6.955903e-02, 8.315175e-01, -6.955903e-02, 6.475365e-02, 6.332111e-02, 
1.726856e-02, 1.976699e-02, 2.384734e-02, 6.935375e-03, -2.240618e-03, 
-2.557954e-03, -2.905944e-03, -1.434360e-03, -2.002750e-04, -9.431106e-04, 
-1.380687e-03, -1.067473e-03, -8.234773e-04}, 
{-3.324008e-04, -4.610215e-04, -9.877869e-04, -6.634257e-04, 3.245128e-04, 
-1.110011e-03, -8.009974e-04, 2.722968e-03, -1.401790e-03, 3.568973e-03, 
2.839553e-02, 1.787064e-02, -4.045942e-03, 6.311557e-02, 8.619662e-02, 
-1.084587e-01, 7.380919e-01, -1.084587e-01, 8.619662e-02, 6.311557e-02, 
-4.045943e-03, 1.787064e-02, 2.839554e-02, 3.568974e-03, -1.401790e-03, 
2.722968e-03, -8.009972e-04, -1.110012e-03, 3.245128e-04, -6.634258e-04, 
-9.877869e-04, -4.610215e-04, -3.324008e-04}, 
{-2.613209e-05, 8.743334e-05, -6.169999e-04, -1.647043e-04, 1.910414e-03, 
-3.281564e-04, -6.732583e-04, 6.721208e-03, 4.028403e-04, -1.110630e-03, 
3.279896e-02, 2.498964e-02, -1.555294e-02, 6.173600e-02, 1.023349e-01, 
-1.459636e-01, 6.505492e-01, -1.459636e-01, 1.023349e-01, 6.173600e-02, 
-1.555294e-02, 2.498964e-02, 3.279896e-02, -1.110630e-03, 4.028402e-04, 
6.721208e-03, -6.732581e-04, -3.281565e-04, 1.910415e-03, -1.647043e-04, 
-6.169999e-04, 8.743333e-05, -2.613209e-05}, 
{2.721962e-04, 6.210879e-04, -1.165285e-04, 3.012638e-04, 3.046174e-03, 
7.467606e-04, 2.545827e-04, 9.843555e-03, 2.530291e-03, -1.799784e-03, 
3.804523e-02, 3.011659e-02, -2.330249e-02, 6.191793e-02, 1.130965e-01, 
-1.751228e-01, 5.855566e-01, -1.751228e-01, 1.130965e-01, 6.191792e-02, 
-2.330249e-02, 3.011659e-02, 3.804524e-02, -1.799784e-03, 2.530291e-03, 
9.843555e-03, 2.545826e-04, 7.467609e-04, 3.046175e-03, 3.012639e-04, 
-1.165285e-04, 6.210878e-04, 2.721962e-04}, 
{2.280648e-04, 8.294150e-04, 3.815275e-05, 3.757086e-04, 3.957605e-03, 
1.638017e-03, 3.297741e-04, 1.261654e-02, 6.788907e-03, -7.865545e-04, 
4.201259e-02, 3.671990e-02, -2.623157e-02, 6.025432e-02, 1.220822e-01, 
-1.831008e-01, 5.584787e-01, -1.831008e-01, 1.220822e-01, 6.025431e-02, 
-2.623158e-02, 3.671990e-02, 4.201259e-02, -7.865546e-04, 6.788907e-03, 
1.261654e-02, 3.297740e-04, 1.638018e-03, 3.957606e-03, 3.757086e-04, 
3.815275e-05, 8.294149e-04, 2.280648e-04}, 
{-5.248226e-05, 5.180685e-04, -3.642315e-04, -9.373923e-05, 3.619876e-03, 
1.162784e-03, -5.173979e-04, 1.232157e-02, 7.164130e-03, -4.473768e-06, 
4.552019e-02, 4.230869e-02, -2.356784e-02, 6.218798e-02, 1.292466e-01, 
-1.737650e-01, 5.667038e-01, -1.737650e-01, 1.292466e-01, 6.218798e-02, 
-2.356784e-02, 4.230870e-02, 4.552019e-02, -4.473775e-06, 7.164129e-03, 
1.232157e-02, -5.173978e-04, 1.162785e-03, 3.619877e-03, -9.373923e-05, 
-3.642315e-04, 5.180685e-04, -5.248226e-05}, 
{-2.666757e-04, 2.524051e-04, -8.515968e-04, -9.164691e-04, 2.618511e-03, 
1.195129e-04, -1.961921e-03, 9.899503e-03, 3.728593e-03, -2.671179e-03, 
4.723991e-02, 4.865862e-02, -1.919687e-02, 6.088794e-02, 1.316148e-01, 
-1.566581e-01, 5.926040e-01, -1.566581e-01, 1.316148e-01, 6.088794e-02, 
-1.919687e-02, 4.865863e-02, 4.723992e-02, -2.671179e-03, 3.728593e-03, 
9.899503e-03, -1.961921e-03, 1.195129e-04, 2.618511e-03, -9.164692e-04, 
-8.515967e-04, 2.524051e-04, -2.666757e-04}, 
{4.399956e-04, 7.865489e-04, -1.473806e-04, 1.805190e-04, 3.453992e-03, 
9.894625e-04, -1.006096e-03, 8.115996e-03, 6.683353e-04, -3.397752e-03, 
4.311076e-02, 4.153124e-02, -1.969737e-02, 5.613427e-02, 1.143282e-01, 
-1.613719e-01, 6.037438e-01, -1.613719e-01, 1.143282e-01, 5.613427e-02, 
-1.969737e-02, 4.153125e-02, 4.311076e-02, -3.397752e-03, 6.683353e-04, 
8.115996e-03, -1.006096e-03, 9.894628e-04, 3.453992e-03, 1.805190e-04, 
-1.473806e-04, 7.865488e-04, 4.399956e-04}, 
{8.945584e-04, 1.072089e-03, 4.867002e-04, 9.993726e-04, 3.161894e-03, 
1.310537e-03, 5.783996e-04, 5.308356e-03, -2.696125e-03, -5.829499e-04, 
3.686139e-02, 2.558001e-02, -1.879127e-02, 6.003916e-02, 9.618367e-02, 
-1.685137e-01, 6.177510e-01, -1.685137e-01, 9.618367e-02, 6.003916e-02, 
-1.879127e-02, 2.558001e-02, 3.686140e-02, -5.829499e-04, -2.696124e-03, 
5.308355e-03, 5.783995e-04, 1.310537e-03, 3.161894e-03, 9.993727e-04, 
4.867002e-04, 1.072088e-03, 8.945584e-04}, 
{8.189208e-04, 7.737336e-04, 4.217418e-04, 7.880542e-04, 1.778679e-03, 
9.313996e-04, 1.351817e-03, 1.868835e-03, -4.775480e-03, 4.729890e-03, 
3.167048e-02, 8.845429e-03, -1.964237e-02, 6.349428e-02, 8.528465e-02, 
-1.562156e-01, 6.573879e-01, -1.562156e-01, 8.528465e-02, 6.349428e-02, 
-1.964237e-02, 8.845430e-03, 3.167048e-02, 4.729890e-03, -4.775480e-03, 
1.868835e-03, 1.351817e-03, 9.313999e-04, 1.778680e-03, 7.880543e-04, 
4.217418e-04, 7.737335e-04, 8.189208e-04}, 
{5.629457e-04, 5.652341e-04, 1.883477e-04, 1.087051e-06, 4.828267e-04, 
8.831835e-04, 1.664209e-03, -1.005162e-04, -4.999327e-03, 5.814011e-03, 
2.455265e-02, 3.093195e-03, -1.484588e-02, 5.120681e-02, 6.386842e-02, 
-1.180820e-01, 7.443071e-01, -1.180820e-01, 6.386842e-02, 5.120681e-02, 
-1.484588e-02, 3.093195e-03, 2.455266e-02, 5.814011e-03, -4.999327e-03, 
-1.005162e-04, 1.664208e-03, 8.831838e-04, 4.828268e-04, 1.087051e-06, 
1.883477e-04, 5.652341e-04, 5.629457e-04}, 
{2.074267e-04, 1.920508e-04, 1.256993e-04, -1.323056e-04, -3.536755e-04, 
4.606532e-04, 1.764229e-03, -4.151823e-04, -3.337312e-03, 4.479203e-03, 
1.298735e-02, -2.083927e-04, -5.235356e-03, 3.044912e-02, 3.070029e-02, 
-6.501155e-02, 8.661319e-01, -6.501155e-02, 3.070029e-02, 3.044912e-02, 
-5.235356e-03, -2.083927e-04, 1.298735e-02, 4.479203e-03, -3.337312e-03, 
-4.151823e-04, 1.764228e-03, 4.606534e-04, -3.536756e-04, -1.323056e-04, 
1.256993e-04, 1.920508e-04, 2.074267e-04}};

/* --------------------------------------------------------------- setfir --- */
/* setfir looks up the coeffs for the particular binaural angle, rho,
 and loads them into the firfilter memory. Also resets hist if needed.
 */
void
setfir(double theta, int nterms, int flag, double *coeffs, double *firtap)
{
    static const double radmax = PI2;  /* 2PI rads */
    double rad_inc, angle, frac;
    register int lower, upper, skip;
        
    /* reset filter histories if flag = 1 */
    
    if (flag && firtap) {
        for (int i = 0; i <= nterms; ++i)
            firtap[i] = 0.0;
    }
    /* calculations to produce interpolated data */
    
    if (coeffs) {
        rad_inc = radmax / NANGLES;  /* distance in rads betw. data pts.  */
        angle = theta / rad_inc;     /* current angle in rad_incs */
        lower = (int)angle;          /* truncate to lower integer */
        frac = angle - lower;        /* for interpolating */
        upper = (lower + 1) % NANGLES;
        
        /* since not all firs use max # of terms stored, here is skip pt. */
        
        skip = (MAXTERMS - nterms) / 2;
        
        /* interpolate and load coefficients */
        for (int i = 0; i < nterms; ++i) {
            int j = i + skip;
            coeffs[i] = FIRDATA[lower][j]
            + (FIRDATA[upper][j] - FIRDATA[lower][j]) * frac;
        }
    }
}

