/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _RTMOUSEPFIELD_H_
#define _RTMOUSEPFIELD_H_

#include <RTcmixMouse.h>
#include <PField.h>
#include <Ougens.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

extern int resetval;		// declared in src/rtcmix/minc_functions.c

typedef enum {
	kRTMouseAxisX,
	kRTMouseAxisY
} RTMouseAxis;

#define LAGFACTOR	0.17
#define MINCF		0.05


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

      // Convert lag, a percentage in range [0, 100], to cutoff frequency,
		// depending on the control rate in effect when this PField was created.
		// Lag pct is inversely proportional to cf, because the lower the cf,
		// the longer the lag time.
      const double nyquist = resetval * 0.5;
		double cf = nyquist * pow(2.0, -(lag * LAGFACTOR));
		if (cf <= MINCF)
			cf = MINCF;
		if (cf > nyquist)
			cf = nyquist;

//#define USEEQ
#ifdef USEEQ
		_filter = new Oequalizer(resetval, OeqLowPass);
		_filter->setparams(cf, 0.5);
#else
		_filter = new Oonepole(resetval, cf);
#endif
	}

	virtual double doubleValue(double dummy) const
	{
		double val = (_axis == kRTMouseAxisX) ? computeValueX() : computeValueY();
		return _filter->next(val);
	}

protected:
	virtual ~RTMousePField() {}

private:
	RTcmixMouse *_mousewin;
	RTMouseAxis _axis;
#ifdef USEEQ
	Oequalizer *_filter;
#else
	Oonepole *_filter;
#endif
	double _min;
	double _default;

	double _diff;
	int _labelID;

	double computeValueX() const
	{
		double val;
		double rawval = _mousewin->getPositionX();
		if (rawval < 0.0)
			val = _default;
		else
			val = _min + (_diff * rawval);
		_mousewin->updateXLabelValue(_labelID, val);
		return val;
	}

	double computeValueY() const
	{
		double val;
		double rawval = _mousewin->getPositionY();
		// NB: roundoff error in getPositionY can cause rawval to be a very
		// small negative number when y-coord is bottom-most of window.
		// A truly invalid value is a much larger negative number.
		if (rawval < -0.000001)
			val = _default;
		else
			val = _min + (_diff * rawval);
		_mousewin->updateYLabelValue(_labelID, val);
		return val;
	}
};

#endif // _RTMOUSEPFIELD_H_
