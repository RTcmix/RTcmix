#include <iostream.h>
#include "../../sys/mixerr.h"
#include "../../rtstuff/Instrument.h"
#include "../../rtstuff/rt.h"
#include "../../rtstuff/rtdefs.h"

void
rtprofile()
{
	RT_INTRO("SFLUTE",makeSFLUTE);
	RT_INTRO("BSFLUTE",makeBSFLUTE);
	RT_INTRO("VSFLUTE",makeVSFLUTE);
	RT_INTRO("LSFLUTE",makeLSFLUTE);
}
