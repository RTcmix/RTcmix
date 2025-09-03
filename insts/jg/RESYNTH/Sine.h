/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _SINE_H_
#define _SINE_H_ 1

// Simple non-interpolating oscillator.  Does not handle negative frequencies!
//
// NB: For the convenience of callers (like LFOPField) who need to decide
// at runtime whether to interpolate, this class provides an interpolating
// lookup method, nexti.  -JGG

#include <stdio.h> //FIXME: tmp

class Sine
{
public:
	Sine(float srate, float freq, float array[], int len);

	float next(void);		// non-interpolating
	float nexti(void);	// linear interpolating

//	inline void setfreq(float freq) { _si = freq * _lendivSR; }
	inline void setfreq(float freq) { _si = freq * _lendivSR; if (_si<0.0) printf("setfreq: _si=%g, freq=%f, _lendivSR=%g\n", _si, freq, _lendivSR);}
	inline void setphase(double phase) { _phase = phase; }
	inline double getphase(void) const { return _phase; }
	inline int getlength(void) const { return _length; }

	// <phase> should be in [-pi, pi]
	void setPhaseRadians(float phase);	// JG added

private:
	double _si, _phase, _lendivSR;
	float *_array;
	int _length;
};

#endif // _SINE_H_
