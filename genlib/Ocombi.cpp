/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <Ocombi.h>
#include <Odelayi.h>
#include <math.h>
#include <assert.h>

Ocombi::Ocombi(float SR, float loopTime, float maxLoopTime, float reverbTime)
	: _sr(SR)
{
	assert(maxLoopTime > 0.0);
	assert(maxLoopTime >= loopTime);

	long maxlen = (long) (maxLoopTime * _sr + 0.5);
	_delay = new Odelayi(maxlen);
	_delsamps = loopTime * _sr;
	_delay->setdelay(_delsamps);
	setReverbTime(reverbTime);
	_lastout = 0.0;
}

Ocombi::~Ocombi()
{
	delete _delay;
}

void Ocombi::clear()
{
	_delay->clear();
	_lastout = 0.0;
};

void Ocombi::setReverbTime(float reverbTime)
{
	assert(reverbTime > 0.0);
	_gain = pow(0.001, (_delsamps / _sr) / reverbTime);
}

float Ocombi::next(float input, float delaySamps)
{
	if (delaySamps != _delsamps) {
		_delsamps = delaySamps;
		_delay->setdelay(_delsamps);
	}
	float tmp = input + (_gain * _delay->last());
	_lastout = _delay->next(tmp);
	return _lastout;
}

