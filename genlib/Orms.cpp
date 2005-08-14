/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <Orms.h>

Orms::Orms(float srate, int windowlen)
	: _counter(0), _windowlen(windowlen), _last(0.0f)
{
	_filter = new Oonepole(srate, 10.0f);
#ifdef ANTI_DENORM
	_antidenorm_offset = 1e-18;	// smallest value observed to be effective
#endif
}

Orms::~Orms()
{
	delete _filter;
}

void Orms::clear()
{
	_last = 0.0f;
	_counter = 0;
	_filter->clear();
}

