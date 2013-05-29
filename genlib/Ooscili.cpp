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
#define kFracMask kFracShift - 1

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

