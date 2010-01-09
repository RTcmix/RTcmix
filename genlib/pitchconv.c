/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// original cmix pitch converters, rev. JGG, 6/20/04
#include <math.h>
#include <ugens.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

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

#define SEMITONE_LINOCT (1.0 / 12.0)

// "let" is ANSI pitch representation (e.g., "Ab3 +22"), as described at 
// http://dactyl.som.ohio-state.edu/Humdrum/representations/pitch.rep.html
// JGG, 1/24/06

INLINE double octlet(unsigned char *let)
{
	//                    A  B   C  D  E  F  G
	int let2semitone[] = {9, 11, 0, 2, 4, 5, 7};
	int octave = -9999;
	int semitones = -9999;
	long cents = 0;
	int state = 0;

	unsigned char *p = let;
	while (*p) {
		if (*p == ' ')                // eat spaces
			p++;
		else {
			if (state == 0) {
				int index = *p - 65;    // 65 is ASCII code for 'A'
				if (index < 0 || index > 6)
					goto err;
				semitones = let2semitone[index];
				p++;
				state++;
			}
			else if (state == 1) {
				if (*p == '#') {        // sharp
					semitones++;
					p++;
				}
				else if (*p == 'b') {   // flat
					semitones--;
					p++;
				}
				else if (*p == 'x') {   // double sharp
					semitones += 2;
					p++;
				}
				state++;
			}
			else if (state == 2) {
				if (*p == 'b') {        // double flat (w/ 'b' from state 1)
					semitones--;
					p++;
				}
				state++;
			}
			else if (state == 3) {
				octave = *p - 48;       // 48 is ASCII code for '0' (zero)
				if (octave < 0 || octave > 9)
					goto err;
				octave += 4;            // ANSI octave to linoct octave
				p++;
				state++;
			}
			else {
				char *pos = NULL;
				errno = 0;
				cents = strtol((char *) p, &pos, 10);
				if (pos == (char *) p || *pos != 0 || errno == ERANGE)
					goto err;
				if (cents < -100 || cents > 100)
					goto err;
				break;
			}
		}
	}
	if (semitones > -4) {   // lowest: "Cbb4 -100" ... dbl flat -100 cents
		double frac = semitones * SEMITONE_LINOCT;
		double linoct = octave + frac;
		if (cents)
			linoct += cents * (SEMITONE_LINOCT * 0.01);
		return linoct;
	}

err:
	// NB: Avoid using die() here to simplify linking for pchcps, etc. utils
	fprintf(stderr, "Invalid pitch representation \"%s\".\n", let);
	return 8.00;
}

INLINE double cpslet(unsigned char *let)
{
	return cpsoct(octlet(let));
}

INLINE double pchlet(unsigned char *let)
{
	return pchoct(octlet(let));
}

