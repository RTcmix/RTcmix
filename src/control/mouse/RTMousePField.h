/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _RTMOUSEPFIELD_H_
#define _RTMOUSEPFIELD_H_

#include <RTcmixMouse.h>
#include <PField.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

typedef enum {
	kRTMouseAxisX,
	kRTMouseAxisY
} RTMouseAxis;


#define ALPHA	-6.0
#define MAXLAG	10.0

// For lag in range [0,MAXLAG], return smoothing coefficient in range [0,1).
static double makeSmoothingCoefficient(double lag)
{
	assert(lag >= 0.0 && lag <= MAXLAG);
	double c = (1.0 - exp(lag * ALPHA / MAXLAG)) / (1.0 - exp(ALPHA));
	if (c > 0.9999)		// coeff must not be 1.0, and best not be too near it
		c = 0.9999;
	return c;
}

class RTMousePField : public RTNumberPField {
public:
	RTMousePField(
			RTcmixMouse			*mousewin,
			const RTMouseAxis	axis,
			const double		minval,
			const double		maxval,
			const double		defaultval,
			const double		lag,
			const char			*prefix,
			const char			*units,
			const int			precision)
		: RTNumberPField(0),
		  _mousewin(mousewin), _axis(axis), _min(minval), _default(defaultval)
	{
		assert(_mousewin != NULL);
//FIXME: set up function pointers for each axis, for use in doubleValue?

		_labelID = -1;
		if (prefix && prefix[0]) {	// no label if null or empty prefix string
			if (_axis == kRTMouseAxisX)
				_labelID = _mousewin->configureXLabel(prefix, units, precision);
			else
				_labelID = _mousewin->configureYLabel(prefix, units, precision);
			if (_labelID == -1)
				fprintf(stderr, "Warning: Max. number of labels already in use.");
		}

		_diff = maxval - minval;
		_val = _smoothval = 0.0;
		_lastRawval = -999.9;	// must be a negative number other than -1.0

		_coeffB = makeSmoothingCoefficient(lag);
		_coeffA = 1.0 - _coeffB;
	}

	virtual double doubleValue(double dummy)
	{
		if (_axis == kRTMouseAxisX)
			computeValueX();
		else
			computeValueY();
		_smoothval = (_coeffA * _val) + (_coeffB * _smoothval);
		return setValue(_smoothval);
	}

protected:
	virtual ~RTMousePField() {}

private:
	RTcmixMouse *_mousewin;
	RTMouseAxis _axis;
	double _min;
	double _default;

	double _diff;
	double _val;
	double _smoothval;
	double _lastRawval;
	double _coeffA;
	double _coeffB;
	int _labelID;

	void computeValueX()
	{
		double rawval = _mousewin->getPositionX();
		if (rawval != _lastRawval) {
			if (rawval < 0.0)
				_val = _default;
			else
				_val = _min + (_diff * rawval);
			_mousewin->updateXLabelValue(_labelID, _val);
			_lastRawval = rawval;
		}
	}

	void computeValueY()
	{
		double rawval = _mousewin->getPositionY();
		if (rawval != _lastRawval) {
			// NB: roundoff error in getPositionY can cause rawval to be a very
			// small negative number when y-coord is bottom-most of window.
			// A truly invalid value is a much larger negative number.
			if (rawval < -0.000001)
				_val = _default;
			else
				_val = _min + (_diff * rawval);
			_mousewin->updateYLabelValue(_labelID, _val);
			_lastRawval = rawval;
		}
	}
};

#endif // _RTMOUSEPFIELD_H_
