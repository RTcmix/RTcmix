/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include <Odelay.h>
#include <stdlib.h>

Odelay::Odelay(long defaultLength) : _dline(NULL), _len(0)
{
	resize(defaultLength);
	_outpoint = 0;
	_inpoint = _len - 1;
}

Odelay::~Odelay()
{
	free(_dline);
}

void Odelay::clear()
{
	for (long i = 0; i < _len; i++)
		_dline[i] = 0.0;
	_lastout = 0.0;
}

void Odelay::putsamp(float samp)
{
	_dline[_inpoint++] = samp;
	if (_inpoint == _len)
		_inpoint = 0;
}

float Odelay::getsamp(double lagsamps)
{
	if (lagsamps >= (double) _len)
		resize((long)(lagsamps + 0.5));
	_outpoint = long(_inpoint - lagsamps - 0.5);
	while (_outpoint < 0)
		_outpoint += _len;
	return _lastout = _dline[_outpoint++];
}

// Set output pointer <_outpoint>.

void Odelay::setdelay(double lagsamps)
{
	if (lagsamps >= (double) _len)
		resize((long)(lagsamps + 0.5));
	_outpoint = long(_inpoint - lagsamps - 0.5);
	while (_outpoint < 0)
		_outpoint += _len;
}

float Odelay::next(float input)
{
	_dline[_inpoint++] = input;
	if (_inpoint == _len)
		_inpoint = 0;
	_lastout = _dline[_outpoint++];
	if (_outpoint == _len)
		_outpoint = 0;
	return _lastout;
}

#include <stdio.h>

static float *newFloats(float *oldptr, long oldlen, long *newlen)
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
			for (long n = oldlen; n < *newlen; ++n)
				ptr[n] = 0.0f;
		}
		else {
			*newlen = oldlen;	// notify caller that realloc failed
			ptr = oldptr;
		}
	}
	return ptr;
}

int Odelay::resize(long thisLength)
{
	// Make a guess at how big the new array should be.
	long newlen = (thisLength < _len * 2) ? _len * 2 : _len + thisLength;
	if (_len > 0)
		printf("Odelay resizing from %ld to %ld\n", _len, newlen);
	_dline = ::newFloats(_dline, _len, &newlen);
	return _len = newlen;
}
