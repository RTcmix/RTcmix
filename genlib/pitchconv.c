/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// original cmix pitch converters, rev. JGG, 6/20/04
#include <math.h>
#include <ugens.h>

// midC freq is 440.0 * pow(2.0, -9.0 / 12.0) = 261.625...
#define MIDC_OFFSET 261.62556530059868 / 256.0
#define LOG_OF_2    0.69314718055994529

double cpsoct(double oct)
{
	return pow(2.0, oct) * MIDC_OFFSET;
}

double octcps(double cps)
{
	return log(cps / MIDC_OFFSET) / LOG_OF_2; 
}

double cpspch(double pch)
{
	double oct = octpch(pch);
	return pow(2.0, oct) * MIDC_OFFSET;
}

double pchcps(double cps)
{
	double oct = octcps(cps);
	return pchoct(oct);
}

double octpch(double pch)
{
	int octave = (int) pch;
	double semitone = (100.0 / 12.0) * (pch - octave);
	return octave + semitone;
}

double pchoct(double oct)
{
	int octave = (int) oct;
	double linsemitone = oct - octave;
	return octave + (0.12 * linsemitone); 
}

double midipch(double pch)
{
	int midinote = (int) (((octpch(pch) - 3.0) * 12.0) + 0.5);
	return midinote;
}

double pchmidi(unsigned char midinote)
{
	return pchoct(((double) midinote / 12.0) + 3.0);
}

