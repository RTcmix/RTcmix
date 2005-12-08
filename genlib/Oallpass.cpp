/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include "Oallpass.h"
#include "Odelay.h"

#include <math.h>
#include <assert.h>

Oallpass::Oallpass(float SR, float loopTime, float reverbTime)
	: _delay(0), _sr(SR), _lastout(0.0)
{
	init(loopTime,
		 loopTime,
		 reverbTime,
		 new Odelay(1 + (long) (loopTime * _sr * 2.0)));
}

Oallpass::Oallpass(float SR, float loopTime, float defaultLoopTime,
			 float reverbTime, Odelay *theDelay)
	: _delay(0), _sr(SR), _lastout(0.0)
{
	init(loopTime,
		 defaultLoopTime,
		 reverbTime, 
		 theDelay ? theDelay : new Odelay(1 + (long) (defaultLoopTime * _sr * 2.0)));
}

void Oallpass::init(float loopTime, float defaultLoopTime, float reverbTime, Odelay *delay)
{
	assert(defaultLoopTime > 0.0);
	assert(defaultLoopTime >= loopTime);
	_delay = delay;
	_delsamps = loopTime * _sr;
	_delay->setdelay(_delsamps);
	setReverbTime(reverbTime);
}

Oallpass::~Oallpass()
{
	delete _delay;
}

void Oallpass::clear()
{
	_delay->clear();
	_lastout = 0.0;
};

void Oallpass::setReverbTime(float reverbTime)
{
	assert(reverbTime > 0.0);
	_gain = pow(0.001, (_delsamps / _sr) / reverbTime);
}

float Oallpass::next(float input)
{
	float temp = _delay->last();
	_lastout = input + (_gain * temp);
	_delay->next(_lastout);
	_lastout = temp - (_gain * _lastout);
	
	return _lastout;
}

float Oallpass::next(float input, float delaySamps)
{
	if (delaySamps != _delsamps) {
		_delsamps = delaySamps;
		_delay->setdelay(_delsamps);
	}

	float temp = _delay->last();
	_lastout = input + (_gain * temp);
	_delay->next(_lastout);
	_lastout = temp - (_gain * _lastout);
	
	return _lastout;
}

float Oallpass::frequency() const
{
	float delay = _delay->delay();
	return (delay > 0.0f) ? _sr / delay : 0.0f;
}

