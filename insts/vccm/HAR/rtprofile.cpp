#include <iostream.h>
#include <mixerr.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>

void
rtprofile()
{
	RT_INTRO("HOLD",makeHOLD);
	RT_INTRO("RELEASE",makeRELEASE);
	RT_INTRO("FADE",makeFADE);
}

