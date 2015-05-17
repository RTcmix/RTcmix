/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// derived from RTcmix Ooscil
#include "Sine.h"
#define NDEBUG
#include <assert.h>

Sine::Sine(float srate, float freq, float array[], int len)
	: _array(array), _length(len)
{
	_lendivSR = (double) _length / srate;
	_si = freq * _lendivSR;
	_phase = 0.0;
}

float Sine::next(void)
{
	int i = (int) _phase;
	assert(i >= 0 && i < _length);
	float output = _array[i];

	// prepare for next call
	_phase += _si;
	while (_phase >= (double) _length)
		_phase -= (double) _length;

	return output;
}

float Sine::nexti(void)
{
	int i = (int) _phase;
	int k = (i + 1) % _length;
	double frac = _phase - (double) i;
	float output = _array[i] + ((_array[k] - _array[i]) * frac);

	// prepare for next call
	_phase += _si;
	while (_phase >= (double) _length)
		_phase -= (double) _length;

	return output;
}

#include <math.h>
#ifndef M_PI
	#define M_PI	3.14159265358979323846264338327950288
#endif
#ifndef TWOPI
	#define TWOPI	(M_PI * 2)
#endif

void Sine::setPhaseRadians(float phase)
{
	// convert to [0,1], then scale to array length
	if (phase < 0.0)
		_phase = ((TWOPI + phase) / TWOPI) * _length;
	else
		_phase = (phase / TWOPI) * _length;
	if (_phase < 0.0)
		_phase = 0.0;
	else if (_phase >= double(_length))
		_phase -= double(_length);
}

