#include <rt.h>

void
rtprofile()
{
	RT_INTRO("START",makeSTART);
	RT_INTRO("BEND",makeBEND);
	RT_INTRO("FRET",makeFRET);
	RT_INTRO("START1",makeSTART1);
	RT_INTRO("BEND1",makeBEND1);
	RT_INTRO("FRET1",makeFRET1);
	RT_INTRO("VSTART1",makeVSTART1);
	RT_INTRO("VFRET1",makeVFRET1);
}
