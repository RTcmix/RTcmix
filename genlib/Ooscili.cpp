#include <ugens.h>
#include "../H/Ougens.h"

Ooscili::Ooscili(float freq, int arr)
{
	array = floc(arr);
	length = fsize(arr);

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
	return(*(array+i) + (*(array+k) - *(array+i)) * frac);
}

void Ooscili::setfreq(float freq)
{
	si = freq * (float)length/SR;
}
