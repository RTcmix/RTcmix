#include <iostream.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "NOISE.h"
#include <rt.h>
#include <rtdefs.h>


extern "C" {
	extern float rsnetc[64][5],amp[64];  /* defined in cfuncs.c */
	extern int nresons;
}

NOISE::NOISE() : Instrument()
{
	// future setup here?
}

NOISE::~NOISE()
{
}


int NOISE::init(float p[], short n_args)
{
// p0 = start; p1 = duration; p2 = amplitude
// p3 = stereo spread (0-1) [optional]
// assumes function table 1 is the amplitude envelope

	int i;

	nsamps = rtsetoutput(p[0], p[1], this);

	amparr = floc(1);
	if (amparr) {
		int lenamp = fsize(1);
		tableset(p[1], lenamp, amptabs);
	}
	else
		advise("NOISE", "Setting phrase curve to all 1's.");

	for(i = 0; i < nresons; i++) {
		myrsnetc[i][0] = rsnetc[i][0];
		myrsnetc[i][1] = rsnetc[i][1];
		myrsnetc[i][2] = rsnetc[i][2];
		myrsnetc[i][3] = myrsnetc[i][4] = 0.0;
		myamp[i] = amp[i];
		}
	mynresons = nresons;

	oamp = p[2];
	skip = (int)(SR/(float)resetval);
	spread = p[3];

	return(nsamps);
}

int NOISE::run()
{
	int i,j;
	float out[2];
	float aamp,val,sig;
	int branch;

	Instrument::run();

	aamp = oamp;           /* in case amparr == NULL */

	branch = 0;
	for (i = 0; i < chunksamps; i++)  {
		if (--branch < 0) {
			if (amparr)
				aamp = tablei(cursamp, amparr, amptabs) * oamp;
			branch = skip;
			}

		sig = rrand();

		out[0] = 0.0;
		for(j = 0; j < mynresons; j++) {
			val = reson(sig, myrsnetc[j]);
			out[0] += val * myamp[j];
			}

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
makeNOISE()
{
	NOISE *inst;

	inst = new NOISE();
	inst->set_bus_config("NOISE");

	return inst;
}
