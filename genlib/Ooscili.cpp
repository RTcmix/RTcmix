/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <ugens.h>
#include <Ougens.h>

Ooscili::Ooscili(float freq, int arr)
{
	array = floc(arr);
	length = fsize(arr);
	init(freq);
}

Ooscili::Ooscili(float freq, float arr[])
{
	array = arr;
	length = sizeof(arr);
	init(freq);
}

Ooscili::Ooscili(float freq, float arr[], int len)
{
	array = arr;
	length = len;
	init(freq);
}

void Ooscili::init(float freq)
{
	lendivSR = (double) length / SR;
	si = freq * lendivSR;
	phase = 0.0;
	dur = 1.0 / freq;	// for arbitrary lookups in the next(nsample) method
}

float Ooscili::next()
{
	int i = (int) phase;
	int k = (i+1) % length;
	double frac = phase - (double) i;
	float output = array[i] + ((array[k] - array[i]) * frac);

	// prepare for next call
	phase += si;
	while (phase >= (double) length)
		phase -= (double) length;
	while (phase < 0.0)
		phase += (double) length;

	return output;
}

float Ooscili::next(int nsample)
{
	double frac = ((double)nsample/SR)/dur * (double)(length-1);
	if (frac >= (double)(length-1))
		return array[length-1];
	int loc1 = (int) frac;
	int loc2 = loc1 + 1;
	frac = frac - (double) loc1;
	return array[loc1] + (frac * (array[loc2] - array[loc1]));
}

