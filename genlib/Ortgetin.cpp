#include <ugens.h>
#include "../H/Ougens.h"

Ortgetin::Ortgetin(Instrument *ins)
{
	int i;

	theInst = ins;
	chns = theInst->inputChannels();
	rsamps = RTBUFSAMPS*chns;
	in = new float[rsamps];

	// force a read for the very first access
	chptr = rsamps;
}

int Ortgetin::next(float *inarr)
{
	int nread = 0;
	int i;

	chptr += chns;
	if (chptr >= rsamps)
	{
		nread = theInst->rtgetin(in, theInst, rsamps);
		chptr = 0;
	}

	for (i = 0; i < chns; i++) inarr[i] = in[chptr+i];

	return(nread);
}
