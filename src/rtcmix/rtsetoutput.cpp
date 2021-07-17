/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <RTcmix.h>
#include <Instrument.h>
#include <ugens.h>
#include <limits.h>

int Instrument::rtsetoutput(float start, float dur, Instrument *theInst)
{
  // DS: Adding check to be sure rtoutput() did not fail.

  if (!RTcmix::outputOpen()) {
  	 die(theInst->name(),
		 "rtsetoutput: No output open for this instrument (rtoutput failed?)!");
	 return -1;
  }
	
	if (start < 0.0f) {
		die(theInst->name(),
			"rtsetoutput: start time must be >= 0.0");
		return -1;
	}
	if (dur < 0.0f) {
		die(theInst->name(),
			"rtsetoutput: duration must be >= 0.0");
		return -1;
	}
  
  theInst->_start = start;
  theInst->_dur = dur;
  theInst->_nsamps = (int)(0.5 + dur * RTcmix::sr());
	if (theInst->_nsamps < 0) {
		rtcmix_warn(theInst->name(), "rtsetoutput: sample count exceeds MAXINT - limiting");
		theInst->_nsamps = INT_MAX;
	}
  
  return 0;
}
