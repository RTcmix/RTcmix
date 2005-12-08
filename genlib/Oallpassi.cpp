/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <Oallpassi.h>
#include <Odelayi.h>

// Oallpassi is now just a specialized version of Oallpass via containment:  Oallpassi
// initializes the base class delay with an interpolated delay.

Oallpassi::Oallpassi(float SR, float loopTime, float defaultLoopTime, float reverbTime)
	: Oallpass(SR, loopTime, defaultLoopTime,
		    reverbTime, new Odelayi(1 + (long) (defaultLoopTime * SR * 2.0)))
{
}


