/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <Ocomb.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

static float *newFloats(float *oldptr, int oldlen, int *newlen)
{
	float *ptr = NULL;
	if (oldptr == NULL) {
		ptr = (float *) malloc(*newlen * sizeof(float));
	}
	else {
		float *newptr = (float *) realloc(oldptr, *newlen * sizeof(float));
		if (newptr) {
			ptr = newptr;
			// Zero out new portion.
			for (int n = oldlen; n < *newlen; ++n)
				ptr[n] = 0.0f;
		}
		else {
			*newlen = oldlen;	// notify caller that realloc failed
			ptr = oldptr;
		}
	}
	return ptr;
}

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
	_dline = newFloats(NULL, 0, &_len);
	clear();
	_delsamps = (int) (loopTime * _sr + 0.5);
	setReverbTime(reverbTime);
	_pointer = 0;
}

Ocomb::~Ocomb()
{
	free(_dline);
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
	if (delaySamps >= _len) {
		// Make a guess at how big the new array should be.
		int newlen = (delaySamps < _len * 2) ? _len * 2 : _len + delaySamps;
		printf("Ocomb resizing from %d to %d\n", _len, newlen);
		_dline = newFloats(_dline, _len, &newlen);
		if (newlen > _len) {
			_len = newlen;
		}
		else {
			printf("Ocomb resize failed!  Limiting delay.\n");
			delaySamps = _len - 1;
		}
	}
	_delsamps = delaySamps;
	if (_pointer >= _delsamps)
		_pointer = 0;
	float out = _dline[_pointer];
	_dline[_pointer++] = (out * _gain) + input;
	return out;
}

