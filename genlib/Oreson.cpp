// RTcmix - Copyright (C) 2005  The RTcmix Development Team
// See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
// the license to this software and for a DISCLAIMER OF ALL WARRANTIES.

#include <Oreson.h>
#include <math.h>

Oreson::Oreson(float srate, float centerFreq, float bandwidth, Scale scale)
	: _h0(0.0f), _h1(0.0f), _last(0.0f)
{
	_a2 = exp(-2.0 * M_PI * bandwidth / srate);
	float temp = 1.0f - _a2;
	float c = _a2 + 1.0f;
	_a1 = 4.0f * _a2 / c * cos(2.0 * M_PI * centerFreq / srate);
	if (scale == kNoScale)
		_a0 = 1.0f;
	else if (scale == kPeakResponse)
		_a0 = temp * sqrt(1.0 - (_a1 * _a1 / (4.0 * _a2)));
	else // kRMSResponse
		_a0 = sqrt(temp / c * ((c * c) - (_a1 * _a1)));
}

