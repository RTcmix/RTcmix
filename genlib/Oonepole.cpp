/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <Oonepole.h>
#include <math.h>
#include <float.h>	// for -FLT_MAX

//#define DEBUG

#ifdef DEBUG
  #include <stdio.h>
  #define DPRINT(msg)			printf((msg))
  #define DPRINT1(msg, arg)	printf((msg), (arg))
#else
  #define DPRINT(msg)
  #define DPRINT1(msg, arg)
#endif

Oonepole::Oonepole(float SR) : _sr(SR), _hist(0.0), _a(0.1), _b(0.9)
{
}

Oonepole::Oonepole(float SR, float freq) : _sr(SR), _hist(0.0)
{
	setfreq(freq);
}

void Oonepole::setfreq(float freq)
{
	DPRINT1("Oonepole::setfreq(%f)\n", freq);

	if (freq >= 0.0) {
		double c = 2.0 - cos(freq * (M_PI * 2.0) / _sr);
		_b = -(sqrt(c * c - 1.0) - c);
	}
	else {
		double c = 2.0 + cos(freq * (M_PI * 2.0) / _sr);
		_b = -(c - sqrt(c * c - 1.0));
	}
	_a = (_b > 0.0) ? 1.0 - _b : 1.0 + _b;
}

// Convert lag, in range [0, 1] to cutoff frequency, and set it.  Lag is
// inversely proportional to cf: the lower the cf, the longer the lag time.

// Empirically determined to offer linear "feel" range.  -JGG
#define LAGFACTOR	12.0
#define MAXCF		500.0

void Oonepole::setlag(float lag)
{
	DPRINT1("Oonepole::setlag(%f)\n", lag);

	double cf = MAXCF * pow(2, -lag * LAGFACTOR);
	if (cf > _sr * 0.5)		// if control rate < 1000
		cf = _sr * 0.5;
	setfreq(cf);
}


// ----------------------------------------------------------- OonepoleTrack ---
// Subclass of Oonepole that tracks changes to freq or lag and performs 
// computations to update them only when they change.  This only works
// if the caller sticks to setfreq or setlag, not mixing calls to both.

OonepoleTrack::OonepoleTrack(float SR) : Oonepole(SR)
{
	_freq = -FLT_MAX;
	_lag = -FLT_MAX;
}

void OonepoleTrack::setfreq(float freq)
{
	DPRINT1("OonepoleTrack::setfreq(%f)\n", freq);

	if (freq != _freq) {
		Oonepole::setfreq(freq);
		_freq = freq;
	}
}

void OonepoleTrack::setlag(float lag)
{
	DPRINT1("OonepoleTrack::setlag(%f)\n", lag);

	if (lag != _lag) {
		Oonepole::setlag(lag);
		_lag = lag;
	}
}

