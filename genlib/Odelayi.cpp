/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include <Odelayi.h>

Odelayi::Odelayi(long defaultLen) : Odelay(defaultLen), _frac(0.0)
{
}

Odelayi::~Odelayi()
{
}

float Odelayi::getsamp(double lagsamps)
{
	_frac = lagsamps - (long) lagsamps;
	const float out = Odelay::getsamp(lagsamps);	// This increments _outpoint
	register float next;
	if (_outpoint < _len)
		next = _dline[_outpoint];
	else
		next = _dline[0];
	return _lastout = out + ((next - out) * _frac);
}

// Set interp fraction <_frac>.  Base class sets output pointer.

void Odelayi::setdelay(double lagsamps)
{
	Odelay::setdelay(lagsamps);
	_frac = lagsamps - (long) lagsamps;
}

float Odelayi::next(float input)
{
	const float out = Odelay::next(input);	// This increments and wraps _outpoint
	return _lastout = (_dline[_outpoint] - out) * _frac;
}

