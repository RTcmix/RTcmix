#include <iostream.h>
#include <mixerr.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>

void
rtprofile()
{
	RT_INTRO("SFLUTE",makeSFLUTE);
	RT_INTRO("BSFLUTE",makeBSFLUTE);
	RT_INTRO("VSFLUTE",makeVSFLUTE);
	RT_INTRO("LSFLUTE",makeLSFLUTE);
}
