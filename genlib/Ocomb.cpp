/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

// Non-interpolating comb filter class.  This produces the buzzy sound
// of the classic cmix comb filter.  Can be used with dynamically changing
// delay times, but with glitchy results -- sometimes desirable!  For
// smoother, duller results, use the Ozcomb object.  Ocomb is a translation
// of cmix combset/comb.  -JGG, 7/8/04

#include <math.h>
#include <ugens.h>      // for SR
#include <Ougens.h>
#include <assert.h>


// Use this when you don't intend to change the loopTime dynamically.
// <reverbTime> must be greater than zero.

Ocomb::Ocomb(float loopTime, float reverbTime)
{
	init(loopTime, loopTime, reverbTime);
}

// Use this when you do intend to change the loopTime dynamically.  Set
// <maxLoopTime> to be the longest delay time (i.e., lowest resonated
// frequency) that you expect to use.  It must be >= loopTime.
// <reverbTime> must be greater than zero.

Ocomb::Ocomb(float loopTime, float maxLoopTime, float reverbTime)
{
	init(loopTime, maxLoopTime, reverbTime);
}

void Ocomb::init(float loopTime, float maxLoopTime, float reverbTime)
{
	assert(maxLoopTime > 0.0);
	assert(maxLoopTime >= loopTime);

	_len = (int) (maxLoopTime * SR + 0.5);
	_dline = new float[_len];
	clear();
	_delsamps = (int) (loopTime * SR + 0.5);
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

// Can be called repeatedly while running.  <reverbTime> must be greater
// than zero.

void Ocomb::setReverbTime(float reverbTime)
{
	assert(reverbTime > 0.0);
	_gain = pow(0.001, ((float) _delsamps / SR) / reverbTime);
}

// There are two next() methods: use the first for non-changing delay
// times, the second for changing delay times.  The second uses a non-
// interpolating delay line, so there will be glitches when changing
// pitch, esp. when glissing downward.

float Ocomb::next(float input)
{
	if (_pointer == _len)
		_pointer = 0;
	float out = _dline[_pointer];
	_dline[_pointer++] = (out * _gain) + input;
	return out;
}

// Note: loopTime is expressed here as delaySamps.
// Make sure <delaySamps> is between 0 and (maxLoopTime * SR), or you'll
// get sudden pitch changes and dropouts.

float Ocomb::next(float input, int delaySamps)
{
	_delsamps = delaySamps;
	if (_pointer >= _delsamps)
		_pointer = 0;
	float out = _dline[_pointer];
	_dline[_pointer++] = (out * _gain) + input;
	return out;
}

