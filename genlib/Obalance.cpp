/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <Obalance.h>

Obalance::Obalance(float srate, int windowlen)
	: _windowlen(windowlen), _increment(0.0f), _last(0.0f), _gain(0.0f)
{
	_inputRMS = new Orms(srate, _windowlen);
	_compareRMS = new Orms(srate, _windowlen);
	_counter = _windowlen + 1;    // sync with Orms "if (--_counter < 0)" blocks
}

Obalance::~Obalance()		
{
	delete _inputRMS;
	delete _compareRMS;
}

void Obalance::clear()
{
	_inputRMS->clear();
	_compareRMS->clear();
	_counter = 0;
	_gain = 0.0f;
	_increment = 0.0f;
	_last = 0.0f;
}

void Obalance::setwindow(const int nframes)
{
	_windowlen = (nframes > 0) ? nframes : 1;
	_counter = _windowlen + 1;    // sync with Orms "if (--_counter < 0)" blocks
	_inputRMS->setwindow(_windowlen);
	_compareRMS->setwindow(_windowlen);
}

