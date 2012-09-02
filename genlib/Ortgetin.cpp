/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <RTcmix.h>
#include <Instrument.h>
#include <Ortgetin.h>

Ortgetin::Ortgetin(Instrument *ins)
{
	theInst = ins;
	chns = theInst->inputChannels();
	rsamps = RTcmix::bufsamps() * chns;
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

	for (i = 0; i < chns; i++)
		inarr[i] = in[chptr+i];

	return nread;
}

