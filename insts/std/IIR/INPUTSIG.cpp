#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "INPUTSIG.h"
#include <rt.h>
#include <rtdefs.h>


extern "C" {
	extern float rsnetc[64][5],amp[64];  /* defined in cfuncs.c */
	extern int nresons;
}

INPUTSIG::INPUTSIG() : Instrument()
{
	in = NULL;
}

INPUTSIG::~INPUTSIG()
{
	delete [] in;
}


int INPUTSIG::init(float p[], int n_args)
{
// p0 = output skip; p1 = input skip; p2 = duration
// p3 = amplitude multiplier; p4 = input channel (0 or 1)
// p5 = stereo spread (0-1) [optional]
// assumes function table 1 is the amplitude envelope

	int i, rvin;

	rvin = rtsetinput(p[1], this);
	if (rvin == -1) { // no input
		return(DONT_SCHEDULE);
	}
	nsamps = rtsetoutput(p[0], p[2], this);

	amparr = floc(1);
	if (amparr) {
		int lenamp = fsize(1);
		tableset(p[2], lenamp, amptabs);
	}
	else
		advise("INPUTSIG", "Setting phrase curve to all 1's.");

	for(i = 0; i < nresons; i++) {
		myrsnetc[i][0] = rsnetc[i][0];
		myrsnetc[i][1] = rsnetc[i][1];
		myrsnetc[i][2] = rsnetc[i][2];
		myrsnetc[i][3] = myrsnetc[i][4] = 0.0;
		myamp[i] = amp[i];
	}
	mynresons = nresons;

	oamp = p[3];
	inchan = (int)p[4];
	if (inchan >= inputchans) {
		die("INPUTSIG", "You asked for channel %d of a %d-channel file.",
                                                        inchan, inputchans);
		return(DONT_SCHEDULE);
	}

	skip = (int)(SR/(float)resetval);
	spread = p[5];

	return(nsamps);
}

int INPUTSIG::run()
{
	int i,j,rsamps;
	float out[2];
	float aamp,val;
	int branch;

	if (in == NULL)        /* first time, so allocate it */
		in = new float [RTBUFSAMPS * inputchans];

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	aamp = oamp;           /* in case amparr == NULL */

	branch = 0;
	for (i = 0; i < rsamps; i += inputchans)  {
		if (--branch < 0) {
			if (amparr)
				aamp = tablei(cursamp, amparr, amptabs) * oamp;
			branch = skip;
			}

		out[0] = 0.0;
		for(j = 0; j < mynresons; j++) {
			val = reson(in[i+inchan],myrsnetc[j]);
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
makeINPUTSIG()
{
	INPUTSIG *inst;

	inst = new INPUTSIG();
	inst->set_bus_config("INPUTSIG");

	return inst;
}
