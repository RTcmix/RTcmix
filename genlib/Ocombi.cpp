/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <Ocombi.h>
#include <Odelayi.h>

// Ocombi is now just a specialized version of Ocomb via containment:  Ocombi
// initializes the base class delay with an interpolated delay.

Ocombi::Ocombi(float SR, float loopTime, float defaultLoopTime, float reverbTime)
	: Ocomb(SR, loopTime, defaultLoopTime,
		    reverbTime, new Odelayi(1 + (long) (defaultLoopTime * SR + 0.5)))
{
}


