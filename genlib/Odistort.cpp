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

float Odistort::SoftClip(float input, float)
{
	return input - (0.33333334 * input * input * input);
}

float Odistort::StrumClip(float input, float)
{
	if (input >= 1.0f)
		return 0.66666667;
	else if (input <= -1.0f)
		return -0.66666667;
	return input - (0.33333334 * input * input * input);
}

float Odistort::SimpleTube(float input, float)
{
// FIXME: not sure this is right  -JGG
	return ((input + 0.5f) * (input + 0.5f)) - 0.25f;
}

float Odistort::VariableClip(float input, float hardness)
{
	double a = pow(fabs(input), hardness);
	double b = pow(atan(a), 1.0 / hardness);
	return sin(input) * b;
}

float Odistort::WaveShape(float input, float a)
{
	float inabs = fabs(input);
	return input * (inabs + a) / ((input * input) + ((a - 1.0) * inabs) + 1.0);
}

