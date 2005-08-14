/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _OBALANCE_H_
#define _OBALANCE_H_ 1

// Adjust amplitude of a signal so that it matches a comparator signal.
// Based on the one in csound.  -JGG, 6/21/05

#include "Orms.h"

class Obalance {

public:
	Obalance(float srate, int windowlen = kDefaultRMSWindowLength);
	~Obalance();

	void setwindow(const int nframes);
	void setgain(const float gain) { _gain = gain; }
	inline float next(float input, float comparator);
	void clear();

private:
	int _counter;
	int _windowlen;
	float _increment;
	float _last;
	float _gain;
	Orms *_inputRMS;
	Orms *_compareRMS;
};


inline float Obalance::next(float input, float comparator)
{
	// Note: must maintain RMS histories even when we don't consult in and cmp
	const float in = _inputRMS->next(input);
	const float cmp = _compareRMS->next(comparator);

	if (--_counter < 0) {
		const float a = in ? cmp / in : cmp;
		const float diff = a - _gain;
		_increment = diff / _windowlen;
		_counter = _windowlen;
	}

	_last = input * _gain;
	_gain += _increment;

	return _last;
}

#endif // _OBALANCE_H_

