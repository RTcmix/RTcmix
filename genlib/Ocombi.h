/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _OCOMBI_H_
#define _OCOMBI_H_ 1

#include "Ocomb.h"

// Interpolating comb filter class.    -JGG, 7/8/04

// Ocombi is now just a specialized version of Ocomb via containment:  Ocombi
// initializes the base class delay with an interpolated delay.	 - D.S. 01/05


class Ocombi : public Ocomb
{
public:

	// Set <defaultLoopTime> to be 1.0/(lowest resonated
	// frequency that you expect to use).  It must be greater than or equal
	// to loopTime.  <reverbTime> must be greater than zero.

	Ocombi(float SR, float loopTime, float defaultLoopTime, float reverbTime);

};

#endif // _OCOMBI_H_
