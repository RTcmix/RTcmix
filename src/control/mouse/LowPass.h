/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _LOWPASS_H_
#define _LOWPASS_H_

#include <math.h>
#include <assert.h>

#define ALPHA	-6.0
#define MAXLAG	10.0

class LowPass {
public:
	LowPass() : _coeffA(0.001), _coeffB(0.999), _hist(0.0) {}

	// For lag in range [0,MAXLAG], return smoothing coefficient in range [0,1).
	void setLag(double lag)
	{
		assert(lag >= 0.0 && lag <= MAXLAG);
		double c = (1.0 - exp(lag * ALPHA / MAXLAG)) / (1.0 - exp(ALPHA));
		if (c > 0.9999)		// coeff must not be 1.0, and best not be too near it
			c = 0.9999;
		_coeffB = c;
		_coeffA = 1.0 - _coeffB;
	}

	inline double next(const double val)
	{
		double newval = (_coeffA * val) + (_coeffB * _hist);
		_hist = newval;
		return newval;
	}

private:
	double	_coeffA;
	double	_coeffB;
	double	_hist;
};

#endif // _LOWPASS_H_

