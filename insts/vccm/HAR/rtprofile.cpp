#include <mixerr.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>

void
rtprofile()
{
	RT_INTRO("STORE",makeSTORE);
	RT_INTRO("HOLD",makeHOLD);
	RT_INTRO("FADE_HOLD",makeFADE_HOLD);
}

