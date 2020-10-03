/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <Ooscili.h>
#include <ugens.h>
//#define NDEBUG
#include <assert.h>

#define kFracBits 16
#define kFracShift 65536
#define kFracMask (kFracShift - 1)

Ooscili::Ooscili(float SR, float freq, int arr) : _sr(SR)
{
	array = floc(arr);
	length = fsize(arr);
	init(freq);
}

Ooscili::Ooscili(float SR, float freq, double arr[], int len) : _sr(SR)
{
	array = arr;
	length = len;
	init(freq);
}

void Ooscili::init(float freq)
{
	assert(length < kFracShift / 2);
	lendivSR = (double) length / _sr;
	si = fp(freq * lendivSR);
	phase = 0;

	// for arbitrary lookups in the next(nsample) method
	tabscale = (double) (length - 1) / (_sr / freq);
}

#include <math.h>
#ifndef M_PI
	#define M_PI	3.14159265358979323846264338327950288
#endif
#ifndef TWOPI
	#define TWOPI	((M_PI) * 2)
#endif

void Ooscili::setPhaseRadians(double phs)
{
	double normphase = 0.0;
	// convert to [0,1], then scale to array length
	if (phs < 0.0)
		normphase = ((TWOPI + phs) / TWOPI) * length;
	else
		normphase = (phs / TWOPI) * length;
	// wrap phase to [0, length)
	if (normphase < 0.0)	{
		while (normphase < 0.0)
			normphase += double(length);
	}
	else {
		while (normphase >= double(length))
			normphase -= double(length);
	}
	phase = fp(normphase);
}

float Ooscili::next()
{
	int i = phase >> kFracBits;
	int k = i + 1;
	if (k >= length)
		k = 0;
	const int frac = phase & kFracMask;
	float output = array[i] + (((array[k] - array[i]) * frac) / kFracShift);

	// prepare for next call
	phase += si;
	fixed_t fplen = length << kFracBits;
	while (phase >= fplen)
		phase -= fplen;
	while (phase < 0)		// handle negative freqs
		phase += fplen;

	return output;
}

float Ooscili::next(int nsample)
{
	assert(nsample >= 0);
	double frac = nsample * tabscale;
	int i = (int) frac;
	if (i >= length - 1)			// NB: we read array[i + 1]
		return array[length - 1];
	frac = frac - (double) i;
	return array[i] + (frac * (array[i + 1] - array[i]));
}

