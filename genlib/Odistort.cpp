/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include "Odistort.h"
#include <math.h>

Odistort::Odistort(Odistort::DistortFunction fun)
	: _fun(fun)
{
}

Odistort::~Odistort()
{
}

// This was in Charlie Sullivan's strum code. Also discussed by
// Julius Smith here:
//    https://ccrma.stanford.edu/~jos/asahb04/Electric_Guitars.html
float Odistort::SoftClip(float input, float)
{
	if (input >= 1.0f)
		return 0.66666667;
	else if (input <= -1.0f)
		return -0.66666667;
	return input - (0.33333334 * input * input * input);
}

float Odistort::SimpleTube(float input, float)
{
	/* From strum:
		Tube-ish distortion: dist = (x +.5)^2 -.25
		Charlie says: 'this does not work with a feedback guitar'
	*/
// FIXME:
	return ((input + 0.5f) * (input + 0.5f)) - 0.25f;
}

/*
Variable-hardness clipping function

References: Posted by Laurent de Soras
[See http://musicdsp.org/files/laurent.gif for an animated graph.]

Notes:
k >= 1 is the "clipping hardness". 1 gives a smooth clipping, and a high value
gives hardclipping.  Don't set k too high, because the formula use the pow()
function, which use exp() and would overflow easily. 100 seems to be a
reasonable value for "hardclipping"

Code:
f(x) = sign(x) * pow(atan(pow(abs(x), k)), (1 / k));
[surely 'sign' should be 'sin' -JG]
*/
float Odistort::VariableClip(float input, float hardness)
{
	double a = pow(fabs(input), hardness);
	double b = pow(atan(a), 1.0 / hardness);
	return sin(input) * b;
}

/*
Type: waveshaper
References: Posted by Bram de Jong

Notes:
where x (in [-1..1] will be distorted and a is a distortion parameter that goes
from 1 to infinity.  The equation is valid for positive and negativ values.  If
a is 1, it results in a slight distortion and with bigger a's the signal get's
more funky.

A good thing about the shaper is that feeding it with bigger-than-one values,
doesn't create strange fx. The maximum this function will reach is 1.2 for a=1.

Code:
f(x,a) = x * (abs(x) + a) / (x^2 + (a - 1) * abs(x) + 1)
*/
float Odistort::WaveShape(float input, float a)
{
	float inabs = fabs(input);
	return input * (inabs + a) / ((input * input) + ((a - 1.0) * inabs) + 1.0);
}

