/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include <Odelayi.h>

Odelayi::Odelayi(long maxLength)
{
	_maxlen = maxLength;
	_dline = new float[_maxlen];
	clear();
	_outpoint = 0;
	_inpoint = _maxlen - 1;
	_frac = 0.0;
}

Odelayi::~Odelayi()
{
	delete [] _dline;
}

void Odelayi::clear()
{
	for (long i = 0; i < _maxlen; i++)
		_dline[i] = 0.0;
	_lastout = 0.0;
}

void Odelayi::putsamp(float samp)
{
	_dline[_inpoint++] = samp;
	if (_inpoint == _maxlen)
		_inpoint = 0;
}

float Odelayi::getsamp(double lagsamps)
{
	double outptr = (double) _inpoint - lagsamps;
	while (outptr < 0.0)
		outptr += (double) _maxlen;
	_outpoint = (long) outptr;
	_frac = outptr - _outpoint;
	_lastout = _dline[_outpoint++];
	if (_outpoint < _maxlen)
		_lastout += (_dline[_outpoint] - _lastout) * _frac;
	else
		_lastout += (_dline[0] - _lastout) * _frac;
	return _lastout;
}

// Set output pointer <_outpoint> and interp fraction <_frac>.

void Odelayi::setdelay(double lagsamps)
{
	double outptr = (double) _inpoint - lagsamps;
	while (outptr < 0.0)
		outptr += (double) _maxlen;
	_outpoint = (long) outptr;
	_frac = outptr - _outpoint;
}

float Odelayi::next(float input)
{
	_dline[_inpoint++] = input;
	if (_inpoint == _maxlen)
		_inpoint = 0;
	_lastout = _dline[_outpoint++];
	if (_outpoint < _maxlen)
		_lastout += (_dline[_outpoint] - _lastout) * _frac;
	else {
		_lastout += (_dline[0] - _lastout) * _frac;
		_outpoint -= _maxlen;
	}
	return _lastout;
}

