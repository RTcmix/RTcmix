/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// Adapted from Dodge and Jerse, and STK OnePole, by JGG

#include <Ougens.h>

Oonepole::Oonepole(float SR) : _sr(SR), _hist(0.0), _a(0.1), _b(0.9)
{
}

Oonepole::Oonepole(float SR, float freq) : _sr(SR), _hist(0.0)
{
	setfreq(freq);
}

// Positive freq gives lowpass; negative freq gives highpass.
void Oonepole::setfreq(float freq)
{
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

