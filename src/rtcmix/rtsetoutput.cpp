/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <globals.h>
#include <ugens.h>
#include <iostream.h>
#include "../rtstuff/Instrument.h"
#include "../rtstuff/rtdefs.h"
#include "../H/dbug.h"
#include <stdlib.h>

extern double schedtime;

int Instrument::rtsetoutput(float start, float dur, Instrument *theInst)
{
// I know this is silly, but I wanted nsamps and I wanted it to
// look like "setnote" in orig cmix insts
  // DJT:  then perhaps we should call it rtsetnote?

  // DS: Adding check to be sure rtoutput() did not fail.

  if (rtfileit < 0) {
  	 die("rtsetoutput", "No output file open for this instrument (rtoutput failed)!");
  }

  // DJT:  made change to increment schedtime here ... not sure how it will work
  if (rtInteractive) {
#ifdef DBUG
    cout << "rtsetoutput():  rtInteractive mode set\n";
#endif
	pthread_mutex_lock(&schedtime_lock);
    start += (float)schedtime;
	pthread_mutex_unlock(&schedtime_lock);
  }
  
  theInst->_start = start;
  theInst->_dur = dur;
  theInst->nsamps = (int)(dur * SR);
  
  return(theInst->nsamps);
}
