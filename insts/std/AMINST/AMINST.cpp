#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <mixerr.h>
#include <Instrument.h>
#include "AMINST.h"
#include <rt.h>
#include <rtdefs.h>


extern "C" {
	#include <ugens.h>
	extern int resetval;
}

AMINST::AMINST() : Instrument()
{
	// future setup here?
}

int AMINST::init(float p[], short n_args)
{
// p0 = start; p1 = duration; p2 = amplitude
// p3 = carrier frequency (hz); p4 = modulator frequency (hz)
// p5 = stereo spread <0-1> [optional]
// assumes function table 1 is the amplitude envelope
// assumes function table 2 is the modulation envelope
// function table 3 is the carrier waveform
// function table 4 is the modulator waveform

	int lenmamp;

	nsamps = rtsetoutput(p[0], p[1], this);

	amparr = floc(1);
	if (amparr) {
		int lenamp = fsize(1);
		tableset(p[1], lenamp, amptabs);
	}
	else
		printf("Setting phrase curve to all 1's\n");

	mamparr = floc(2);
	lenmamp = fsize(2);
	tableset(p[1], lenmamp, mamptabs);

	cartable = floc(3);
	lencar = fsize(3);
	sicar = p[3] * (float)lencar/SR;

	modtable = floc(4);
	lenmod = fsize(4);
	simod = p[4] * (float)lenmod/SR;

	amp = p[2];
	skip = (int)(SR/(float)resetval);
	phasecar = phasemod = 0.0;
	spread = p[5];

	return(nsamps);
}

int AMINST::run()
{
	int i;
	float out[2];
	float aamp,maamp;
	float tval1,tval2;
	int branch;

	Instrument::run();

	aamp = amp;        /* in case amparr == NULL */

	branch = 0;
	for (i = 0; i < chunksamps; i++)  {
		if (--branch < 0) {
			if (amparr)
				aamp = tablei(cursamp, amparr, amptabs) * amp;
			maamp = tablei(cursamp,mamparr,mamptabs);
			branch = skip;
			}

		tval1 = oscili(1.0, sicar, cartable, lencar, &phasecar);
		tval2 = tval1 * oscili(1.0, simod, modtable, lenmod, &phasemod);
		out[0] = (tval1*(1.0-maamp)) + (tval2*maamp);
		out[0] *= aamp;

		if (outputchans == 2) {
			out[1] = out[0] * (1.0 - spread);
			out[0] *= spread;
			}

		rtaddout(out);
		cursamp++;
		}
	return(i);
}



Instrument*
makeAMINST()
{
	AMINST *inst;

	inst = new AMINST();
	inst->set_bus_config("AMINST");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("AMINST",makeAMINST);
}

