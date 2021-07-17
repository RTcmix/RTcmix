#include <rt.h>

#ifndef EMBEDDED
void
rtprofile()
{
	RT_INTRO("SFLUTE",makeSFLUTE);
	RT_INTRO("BSFLUTE",makeBSFLUTE);
	RT_INTRO("VSFLUTE",makeVSFLUTE);
	RT_INTRO("LSFLUTE",makeLSFLUTE);
}
#endif
