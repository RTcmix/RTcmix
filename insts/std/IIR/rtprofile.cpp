#include <mixerr.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>

void
rtprofile()
{
	RT_INTRO("INPUTSIG",makeINPUTSIG);
	RT_INTRO("NOISE",makeNOISE);
	RT_INTRO("BUZZ",makeBUZZ);
	RT_INTRO("PULSE",makePULSE);
}

