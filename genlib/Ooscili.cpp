#include <ugens.h>
#include <Ougens.h>

Ooscili::Ooscili(float freq, int arr)
{
	array = floc(arr);
	length = fsize(arr);

	si = freq * (float)length/SR;
	phase = 0.0;
	dur = 1.0/freq; // for arbitrary lookups in the next(nsample) method
}

Ooscili::Ooscili(float freq, float arr[])
{
	array = arr;
	length = sizeof(arr);

	si = freq * (float)length/SR;
	phase = 0.0;
}

Ooscili::Ooscili(float freq, float arr[], int len)
{
	array = arr;
	length = len;

	si = freq * (float)length/SR;
	phase = 0.0;
}

float Ooscili::next()
{
	int i,k;
	float frac;

	i = (int)phase;
	k = (i+1) % length;
	frac = phase - (float)i;
	phase += si;
	while (phase >= (float)length) phase -= (float)length;
	return (array[i] + (array[k] - array[i]) * frac);
}

float Ooscili::next(int nsample)
{
	int loc1, loc2;
	float frac;

	frac = ((float)nsample/SR)/dur * (float)(length-1);
	if (frac >= (float)(length-1)) return(array[length-1]);
	loc1 = (int)frac;
	loc2 = loc1+1;
	frac = frac - (float)loc1;
	return(array[loc1] + frac * (array[loc2] - array[loc1]));
}

void Ooscili::setfreq(float freq)
{
	si = freq * (float)length/SR;
}

void Ooscili::setphase(float phs)
{
	phase = phs;
}

int Ooscili::getlength()
{
	return length;
}

float Ooscili::getdur()
{
	return dur;
}
