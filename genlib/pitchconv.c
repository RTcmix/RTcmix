/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// original cmix pitch converters, rev. JGG, 6/20/04
#include <math.h>
#include <ugens.h>

#ifdef __GNUC__
#define INLINE inline
#else
#define INLINE
#endif

// midC freq is 440.0 * pow(2.0, -9.0 / 12.0) = 261.625...
#define MIDC_OFFSET (261.62556530059868 / 256.0)

#ifndef M_LN2		// log_e 2
	#define M_LN2	0.69314718055994529
#endif

INLINE double cpsoct(double oct)
{
	return pow(2.0, oct) * MIDC_OFFSET;
}

INLINE double octcps(double cps)
{
	return log(cps / MIDC_OFFSET) / M_LN2;
}

INLINE double octpch(double pch)
{
	int octave = (int) pch;
	double semitone = (100.0 / 12.0) * (pch - octave);
	return octave + semitone;
}

INLINE double cpspch(double pch)
{
	double oct = octpch(pch);
	return cpsoct(oct);
}

INLINE double pchoct(double oct)
{
	int octave = (int) oct;
	double linsemitone = oct - octave;
	return octave + (0.12 * linsemitone); 
}

INLINE double pchcps(double cps)
{
	double oct = octcps(cps);
	return pchoct(oct);
}

INLINE double midipch(double pch)
{
	int midinote = (int) (((octpch(pch) - 3.0) * 12.0) + 0.5);
	return midinote;
}

INLINE double pchmidi(unsigned char midinote)
{
	return pchoct(((double) midinote / 12.0) + 3.0);
}

INLINE double octmidi(unsigned char midinote)
{
	return ((double) midinote / 12.0) + 3.0;
}

