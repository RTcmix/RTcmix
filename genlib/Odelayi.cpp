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
	// This call increments _outpoint
//	const float out = Odelay::getsamp(lagsamps + 1);
	const float out = Odelay::getsamp(lagsamps);
	register float next;
	if (_outpoint < _len)
		next = _dline[_outpoint];
	else
		next = _dline[0];
	return _lastout = next - _frac * (next - out);
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
	const float next = _dline[_outpoint];
	return _lastout = next - _frac * (next - out);
}

float Odelayi::delay() const
{
	return Odelay::delay() + _frac;
}
