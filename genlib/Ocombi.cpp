/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

// Interpolating comb filter class.    -JGG, 7/8/04

#include <math.h>
#include <ugens.h>
#include <Ougens.h>
#include <assert.h>


// Set <maxLoopTime> to be the longest delay time (i.e., lowest resonated
// frequency) that you expect to use.  It must be >= loopTime.  <reverbTime>
// must be greater than zero.

Ozcomb::Ozcomb(float loopTime, float maxLoopTime, float reverbTime)
{
	assert(maxLoopTime > 0.0);
	assert(maxLoopTime >= loopTime);

	long maxlen = (long) (maxLoopTime * SR + 0.5);
	_delay = new Ozdelay(maxlen);
	_delsamps = loopTime * SR;
	_delay->setdelay(_delsamps);
	setReverbTime(reverbTime);
	_lastout = 0.0;
}

Ozcomb::~Ozcomb()
{
	delete _delay;
}

void Ozcomb::clear()
{
	_delay->clear();
	_lastout = 0.0;
};

// Can be called repeatedly while running.  <reverbTime> must be greater
// than zero.

void Ozcomb::setReverbTime(float reverbTime)
{
	assert(reverbTime > 0.0);
	_gain = pow(0.001, (_delsamps / SR) / reverbTime);
}

// Make sure <delaySamps> is between 0 and (maxLoopTime * SR), or you'll
// get sudden pitch changes and dropouts.

float Ozcomb::next(float input, float delaySamps)
{
	if (delaySamps != _delsamps) {
		_delsamps = delaySamps;
		_delay->setdelay(_delsamps);
	}
	float tmp = input + (_gain * _delay->last());
	_lastout = _delay->next(tmp);
	return _lastout;
}

