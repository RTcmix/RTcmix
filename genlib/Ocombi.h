/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _OCOMBI_H_
#define _OCOMBI_H_ 1

// Interpolating comb filter class.    -JGG, 7/8/04

class Odelayi;

class Ocombi
{
public:

	// Set <maxLoopTime> to be the longest delay time (i.e., lowest resonated
	// frequency) that you expect to use.  It must be greater than or equal
	// to loopTime.  <reverbTime> must be greater than zero.

	Ocombi(float SR, float loopTime, float maxLoopTime, float reverbTime);

	~Ocombi();
	void clear();

	// setReverbTime can be called repeatedly while running.
	// <reverbTime> must be greater than zero.

	void setReverbTime(float reverbTime);

	// Note: loopTime is expressed here as delaySamps.
	// Make sure <delaySamps> is between 0 and (maxLoopTime * _sr), or you'll
	// get sudden pitch changes and dropouts.

	float next(float input, float delaySamps);

private:
	Odelayi *_delay;
	float _sr;
	float _gain;
	float _lastout;
	float _delsamps;
};

#endif // _OCOMBI_H_
