/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _OCOMB_H_
#define _OCOMB_H_ 1

// Non-interpolating comb filter class.  This produces the buzzy sound
// of the classic cmix comb filter.  Can be used with dynamically changing
// delay times, but with glitchy results -- sometimes desirable!  For
// smoother, duller results, use the Ozcomb object.  Ocomb is a translation
// of cmix combset/comb.  -JGG, 7/8/04

class Ocomb
{
public:

	// Use this constructor when you don't intend to change the loopTime
	// dynamically.  <reverbTime> must be greater than zero.

	Ocomb(float SR, float loopTime, float reverbTime);

	// Use this constructor when you do intend to change the loopTime
	// dynamically.  Set <maxLoopTime> to be the longest delay time (i.e.,
	// lowest resonated frequency) that you expect to use.  It must be greater
	// than or equal to loopTime.  <reverbTime> must be greater than zero.

	Ocomb(float SR, float loopTime, float maxLoopTime, float reverbTime);

	~Ocomb();
	void clear();

	// setReverbTime can be called repeatedly while running.
	// <reverbTime> must be greater than zero.

	void setReverbTime(float reverbTime);

	// There are two next() methods: use the first for non-changing delay
	// times, the second for changing delay times.  The second uses a non-
	// interpolating delay line, so there will be glitches when changing
	// pitch, esp. when glissing downward.

	float next(float input);

	// Note: loopTime is expressed here as delaySamps.
	// Make sure <delaySamps> is between 0 and (maxLoopTime * _sr), or you'll
	// get sudden pitch changes and dropouts.

	float next(float input, int delaySamps);

private:
	void init(float loopTime, float maxLoopTime, float reverbTime);
	float _sr;
	float *_dline;
	int _len;
	int _delsamps;
	float _gain;
	int _pointer;
};

#endif // _OCOMB_H_
