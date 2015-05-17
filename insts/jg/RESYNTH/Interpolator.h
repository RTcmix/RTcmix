// Copyright (C) 2012 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#ifndef _INTERPOLATOR_H_
#define _INTERPOLATOR_H_

// NB: There is only a header file for this class, no .cpp.

// currently limited to linear interpolation
class Interpolator {
public:
	Interpolator(void)
		: _interpSamps(10), _counter(10), _increment(0.0),
		  _curval(0.0), _target(0.0) {}

	// Set the first value that will be returned by next().
	void init(float initval) { _curval = (double) initval; }

	// How many samples to interpolate over?
	void samps(const int samps) { _interpSamps = samps; }

	// Value to reach at end of interpolation. This can be called while
	// next() is still interpolating to the previous target, but hasn't
	// yet reached it.
	void target(const float target) {
		_target = target;
		_increment = (_target - _curval) / _interpSamps;
		_counter = _interpSamps;
	}
	float target(void) const { return _target; }

	float next(void) {
		float val = _curval;
		if (_counter > 0) {
			_curval += _increment;
			_counter--;
		}
		else
			_curval = _target;
		return val;
	}

private:
	int		_interpSamps, _counter;
	double	_increment, _curval, _target;
};

#endif // _INTERPOLATOR_H_
