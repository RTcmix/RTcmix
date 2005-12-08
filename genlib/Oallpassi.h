/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _OallpassI_H_
#define _OallpassI_H_ 1

#include "Oallpass.h"

// Interpolating allpass filter class.  -JMB, 11/27/05
// Oallpassi initializes the base class delay with an interpolated delay.

class Oallpassi : public Oallpass
{
public:

	// Set <defaultLoopTime> to be 1.0/(lowest resonated
	// frequency that you expect to use).  It must be greater than or equal
	// to loopTime.  <reverbTime> must be greater than zero.

	Oallpassi(float SR, float loopTime, float defaultLoopTime, float reverbTime);

};

#endif // _OallpassI_H_
