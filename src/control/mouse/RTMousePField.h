/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _RTMOUSEPFIELD_H_
#define _RTMOUSEPFIELD_H_

#include <RTcmixMouse.h>
#include <PField.h>
#include <stdio.h>
#include <assert.h>

typedef enum {
	RTMouseAxisX,
	RTMouseAxisY
} RTMouseAxis;

class RTMousePField : public RTNumberPField {
public:
	RTMousePField(
			RTcmixMouse			*mousewin,
			const RTMouseAxis	axis,
			const char			*prefix,
			const char			*units,
			const int			precision,
			const double		minval,
			const double		maxval,
			const double		defaultval)
		: RTNumberPField(0),
		  _mousewin(mousewin), _axis(axis), _min(minval), _default(defaultval)
	{
		assert(_mousewin != NULL);
//FIXME: replace with function pointers for each axis (and in doubleValue)
		if (_axis == RTMouseAxisX)
			_labelID = _mousewin->configureXLabel(prefix, units, precision);
		else
			_labelID = _mousewin->configureYLabel(prefix, units, precision);
		if (_labelID == -1)
			fprintf(stderr, "Warning: label setup failed.");
		_diff = maxval - minval;
		_val = 0.0;
		_lastRawval = -999.9;	// must be a negative number other than -1.0
	}

	virtual double doubleValue()
	{
		return (_axis == RTMouseAxisX) ? doubleValueX() : doubleValueY();
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
	double _lastRawval;
	int _labelID;

	double doubleValueX()
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
		return setValue(_val);
	}

	double doubleValueY()
	{
		double rawval = _mousewin->getPositionY();
		if (rawval != _lastRawval) {
			if (rawval < 0.0)
				_val = _default;
			else
				_val = _min + (_diff * rawval);
			_mousewin->updateYLabelValue(_labelID, _val);
			_lastRawval = rawval;
		}
		return setValue(_val);
	}
};

#endif // _RTMOUSEPFIELD_H_
