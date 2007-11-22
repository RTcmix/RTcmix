/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <Odcblock.h>

Odcblock::Odcblock()
	: _hist(0.0), _last(0.0), _coeff(0.99)
{
}

Odcblock::~Odcblock()
{
}

