/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <Ocomb.h>
#include <math.h>
#include <assert.h>

Ocomb::Ocomb(float SR, float loopTime, float reverbTime) : _sr(SR)
{
	init(loopTime, loopTime, reverbTime);
}

Ocomb::Ocomb(float SR, float loopTime, float maxLoopTime, float reverbTime)
	: _sr(SR)
{
	init(loopTime, maxLoopTime, reverbTime);
}

void Ocomb::init(float loopTime, float maxLoopTime, float reverbTime)
{
	assert(maxLoopTime > 0.0);
	assert(maxLoopTime >= loopTime);

	_len = (int) (maxLoopTime * _sr + 0.5);
	_dline = new float[_len];
	clear();
	_delsamps = (int) (loopTime * _sr + 0.5);
	setReverbTime(reverbTime);
	_pointer = 0;
}

Ocomb::~Ocomb()
{
	delete [] _dline;
}

void Ocomb::clear()
{
	for (int i = 0; i < _len; i++)
		_dline[i] = 0.0;
}

void Ocomb::setReverbTime(float reverbTime)
{
	assert(reverbTime > 0.0);
	_gain = pow(0.001, ((float) _delsamps / _sr) / reverbTime);
}

float Ocomb::next(float input)
{
	if (_pointer == _len)
		_pointer = 0;
	float out = _dline[_pointer];
	_dline[_pointer++] = (out * _gain) + input;
	return out;
}

float Ocomb::next(float input, int delaySamps)
{
	_delsamps = delaySamps;
	if (_pointer >= _delsamps)
		_pointer = 0;
	float out = _dline[_pointer];
	_dline[_pointer++] = (out * _gain) + input;
	return out;
}

