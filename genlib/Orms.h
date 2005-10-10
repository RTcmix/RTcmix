/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _ORMS_H_
#define _ORMS_H_ 1

// Compute the RMS power of a signal over a certain number of samples.
// -JGG, 6/21/05

#include <math.h>
#include "Oonepole.h"

#if defined(i386)
 #define ANTI_DENORM
#endif

#ifndef sqrtf
	#define sqrtf(val) sqrt((val))
#endif

const int kDefaultRMSWindowLength = 200;

class Orms {

public:
	Orms(float srate, int windowlen = kDefaultRMSWindowLength);
	~Orms();

	void setwindow(const int nframes) { _windowlen = nframes; _counter = 0; }
	inline float next(float sig);
	void clear();

private:
	int _counter;
	int _windowlen;
	float _last;
#ifdef ANTI_DENORM
	float _antidenorm_offset;
#endif
	Oonepole *_filter;
};


inline float Orms::next(float sig)
{
#ifdef ANTI_DENORM
	// Without this, squaring sig is *very* slow if it's a denormal.
	sig += _antidenorm_offset;
	_antidenorm_offset = -_antidenorm_offset;
#endif
	float tmp = _filter->next(sig * sig);
	if (--_counter < 0) {
		_last = sqrtf(tmp);
		_counter = _windowlen;
	}
	return _last;
}

#endif // _ORMS_H_

