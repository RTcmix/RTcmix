#include <iostream.h>
#include "../rtstuff/Instrument.h"
#include "../rtstuff/rtdefs.h"
#include "../H/dbug.h"

extern int rtInteractive;
extern double schedtime;

int rtsetoutput(float start, float dur, Instrument *theInst)
{
// I know this is silly, but I wanted nsamps and I wanted it to
// look like "setnote" in orig cmix insts
  // DJT:  then perhaps we should call it rtsetnote?
  // DJT:  made change to increment schedtime here ... not sure how it will work
  if (rtInteractive) {
#ifdef DBUG
    cout << "rtsetoutput():  rtInteractive mode set\n";
#endif
    start += (float)schedtime;
  }
  
  theInst->start = start;
  theInst->dur = dur;
  theInst->nsamps = dur * SR;
  
  return(theInst->nsamps);
}
