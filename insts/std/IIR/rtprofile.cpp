#include <iostream.h>
#include "../../sys/mixerr.h"
#include "../../rtstuff/Instrument.h"
#include "../../rtstuff/rt.h"
#include "../../rtstuff/rtdefs.h"

void
rtprofile()
{
	RT_INTRO("INPUTSIG",makeINPUTSIG);
	RT_INTRO("NOISE",makeNOISE);
	RT_INTRO("BUZZ",makeBUZZ);
	RT_INTRO("BUZZ",makeBUZZ);
	RT_INTRO("PULSE",makePULSE);
}

