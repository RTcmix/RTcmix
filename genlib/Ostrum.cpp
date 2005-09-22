/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include "Ostrum.h"
#include <math.h>

const float Ostrum::kMinFreq = 20.0f;

// NB: Comments and variable names derived from Charlie's C strum code,
// to make comparisons between that code and this object easier.  -JGG

Ostrum::Ostrum(float srate, float freq, int squish, float fundDecayTime,
	float nyquistDecayTime)
	: _srate(srate), _funddcy(fundDecayTime), _nyqdcy(nyquistDecayTime),
	  _dcz1(0.0f)
{
	// Size strum array so that it will work with freqs as low as kMinFreq. -JGG
	_dlen = int((1.0 / kMinFreq * _srate) + 0.5);
	_d = new float [_dlen];

	_maxfreq = _srate * 0.333333f;	// Prevent _d underflow.  -JGG

	sset(freq, fundDecayTime, nyquistDecayTime);

	// Init this outside of sset, which may be called to change freq.  -JGG
	_p = _n;

	randfill();
	squisher(squish);
}


Ostrum::~Ostrum()
{
	delete [] _d;
}


// sset sets up the strum array <_d> and other variables for Ostrum to use as
// a plucked string.  Uses a two-point averaging filter to adjust the phase for
// correct pitch, and then uses a linear-phase three-point averaging filter to
// adjust the fundamental frequency to decay in time <fundDecayTime>, and in
// time <nyquistDecayTime> at the Nyquist frequency.  In some cases, the decay
// time at the Nyquist frequency (which always must be less than that at the
// fundamental) will not be exactly as requested, but a wide range of variation
// is possible.  The two-point and three-point filters are combined into a
// single four-point filter.  A single-pole DC-blocking filter is added,
// because the four-point filter may have some gain at DC.  This function may
// be used to change frequency during the course of a note.

void Ostrum::sset(float freq, float fundDecayTime, float nyquistDecayTime)
{
	if (freq < kMinFreq)
		freq = kMinFreq;
	else if (freq > _maxfreq)
		freq = _maxfreq;

	float xlen = _srate / freq;
	double w0 = freq / (_srate * M_PI * 2.0);

	// ncycles is the number of cycles to decay
	float ncycles0 = freq * fundDecayTime;
	float ncyclesNy = freq * nyquistDecayTime;
	float dH0 = pow(0.1, 1.0 / ncycles0);	// level will be down to -20dB after t
	float dHNy = pow(0.1, 1.0 / ncyclesNy);

	float del = 1.0;	// delay of 1 from three-point filter to be added later
	_n = int(floor(xlen - del));
	if (_n > _dlen)
		_n = _dlen;

	float xerr = _n - xlen + del;		// xerr will be a negative number

	// Calculate the phase shift needed from two-point averaging filter.
	// Calculate the filter coefficient c1:  y = c1 * xn + (1 - c1) * x(n - 1)
	float tgent = tan(xerr * w0);		// tan of theta
	float c = cos(w0);
	float s = sin(w0);
	float c1 = (-s - (c * tgent)) / (tgent * (1.0 - c) - s);
	float c2 = 1.0 - c1;

	// effect of this filter on amplitude response
	float H01 = sqrt((c2 * c2 * s * s)
							+ (c1 * (1.0 - c) + c) * (c1 * (1.0 - c) + c));
	float HNy1 = fabs((2.0 * c1) - 1.0);

	// Now add three-point linear phase averaging filter with delay of 1,
	// y = xn*a0 + xn-1*a1 + xn-2*a0
	// and a gain or loss factor, g, so that the filter*g has response H02 and
	// HNy2 to make the total response of all the filters dH0 and dHNy.
	float H02 = dH0 / H01;
	float HNy2 = (HNy1 > 0.0) ? dHNy / HNy1 : 1.e10;

	float g = ((2.0 * H02) - ((1.0 - c) * HNy2)) / (1.0 + c);
	float a1 = ((HNy2 / g) + 1.0) / 2.0;
	// float a1 = ((H02 / g) - c) / (1.0 - c); // alternate equivalent expression

	// For this filter to be monotonic low pass, a1 must be between .5 and 1.
	// If it isn't, response at Nyquist won't be as specified, but it will be
	// set as close as is feasible.
	if (a1 < 0.5) {
		// too fast a Nyquist decay requested
		a1 = 0.5;
		float H = ((1.0 - a1) * c) + a1;
		g = H02 / H;
	}
	else if (a1 > 1.0) {
		// too slow a Nyquist decay requested
		a1 = 1.0;
		g = H02;
	}

	float a0 = (1.0 - a1) / 2.0;
	a0 *= g;
	a1 *= g;

	// Combine the two and three-point averaging filters into one four-point
	// filter with coefficients _a0, _a1, _a2, _a3.
	_a0 = a0 * c1;
	_a1 = (a0 * c2) + (a1 * c1);
	_a2 = (a0 * c1) + (a1 * c2);
	_a3 = a0 * c2;

	// Set up DC-blocking filter.
	float temp = M_PI * (freq / 18.0 / _srate);	// cf at freq / 18
	_dca0 = 1.0 / (1.0 + temp);
	_dca1 = -_dca0;
	_dcb1 = _dca0 * (1.0 - temp);
}


// Fill plucked string array <_d> with white noise.

#include "Orand.h"

void Ostrum::randfill()
{
	// Set values of array to zero.
	for (int i = 0; i < _dlen; i++)
		_d[i] = 0.0f;

	// Fill with white noise.
	Orand randgen;
	float total = 0.0f;
	for (int i = 0; i < _n; i++) {
		_d[i] = randgen.rand();
		total += _d[i];
	}

	// Subtract any DC component.
	float average = total / float(_n);

	for (int i = 0; i < _n; i++)
		_d[i] -= average;
}


// Squish models the softness of a plucking implement by filtering the values
// put in the strum array with an averaging filter.  The filter makes <squish>
// passes.  The loss of amplitude at the fundamental frequency is compensated
// for, but the overall amplitude of the squished string is lowered, as the
// energy at other frequencies is decreased.

void Ostrum::squisher(int squish)
{
	int p1 = _n - 1;
	int p2 = _n - 2;

	float mult = fabs(1.0 / (2.0 * cos(2.0 * M_PI / _n) + 1.0));
	if (mult > 0.5f)	// JGG: avoid huge amps for very low _n
		mult = 0.5f;

	for (int j = 0; j < squish; j++) {
		for (int i = 0; i < _n; i++) {
			_d[i] = mult * (_d[p2] + _d[i] + _d[p1]);
			p2 = p1;
			p1 = i;
		}
	}
}


float Ostrum::next(float input)
{
	if (++_p >= _dlen)
		_p = 0;

	int p1 = _p - _n;
	if (p1 < 0)
		p1 = p1 + _dlen;
	int p2 = p1 - 1;
	if (p2 < 0)
		p2 = p2 + _dlen;
	int p3 = p2 - 1;
	if (p3 < 0)
		p3 = p3 + _dlen;
	int p4 = p3 - 1;
	if (p4 < 0)
		p4 = p4 + _dlen;

	// four-point averaging filter
	float x = _a3 * _d[p4];
	x += _a2 * _d[p3];
	x += _a1 * _d[p2];
	x += _a0 * _d[p1];

	// DC-blocking filter
	_d[_p] = _dca1 * _dcz1;
	_dcz1 = (_dcb1 * _dcz1) + input + x;
	_d[_p] += _dca0 * _dcz1;

	return _d[_p];
}

