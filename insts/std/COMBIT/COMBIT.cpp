#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "COMBIT.h"
#include <rt.h>
#include <rtdefs.h>
#include <combs.h>


COMBIT::COMBIT() : Instrument()
{
    in = NULL;
    combarr = NULL;
}

COMBIT::~COMBIT()
{
    delete [] in;
    delete [] combarr;
}


int COMBIT::init(float p[], int n_args)
{
// p0 = outsk; p1 = insk; p2 = input dur; p3 = smplitude multiplier
// p4 = pitch (cps); p5 = reverb time; p6 = input channel [optional]
// p7 = stereo spread [optional]
// assumes function table 1 is the amplitude envelope

	float loopt;
	int rvin;

	rvin = rtsetinput(p[1], this);
	if (rvin == -1) { // no input
		return(DONT_SCHEDULE);
	}
	nsamps = rtsetoutput(p[0], p[2]+p[5], this);
	insamps = (int)(p[2] * SR);

	if (p[4] < 0.01) {
		die("COMBIT", "Invalid pitch value!");
		return(DONT_SCHEDULE);
	}
	loopt = 1.0/p[4];

	// adding "START" didn't do it, but adding 10.0 seems
	// to prevent the array out-of-bounds bug
	// BGG
	combarr = new float[int(loopt * SR + 10.0)];
	if (!combarr) {
		die("COMBIT", "Could not allocate memory for comb array!");
		return(DONT_SCHEDULE);
	}

	combset(loopt,p[5],0,combarr);

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(p[2]+p[5], amplen, tabs);
	}
	else
		advise("COMBIT", "Setting phrase curve to all 1's.");

	amp = p[3];
	skip = (int)(SR/(float)resetval); // how often to update amp curve
	inchan = (int)p[6];
	if ((inchan+1) > inputchans) {
		die("COMBIT", "You asked for channel %d of a %d-channel file.", 
                                                         inchan,inputchans);
		return(DONT_SCHEDULE);
	}

	spread = p[7];

	return(nsamps);
}

int COMBIT::run()
{
	int i,j,rsamps;
	float out[2];
	float aamp;
	int branch;

	if (in == NULL)    /* first time, so allocate it */
		in = new float [RTBUFSAMPS * inputchans];

	Instrument::run();

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	aamp = amp;        /* in case amptable == NULL */

	branch = 0;
	for (i = 0; i < rsamps; i += inputchans)  {
		if (cursamp > insamps) {
			for (j = 0; j < inputchans; j++) in[i+j] = 0.0;
			}

		if (--branch < 0) {
			if (amptable)
				aamp = table(cursamp, amptable, tabs) * amp;
			branch = skip;
			}

		out[0] = comb(in[i+inchan],combarr) * aamp;
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
makeCOMBIT()
{
	COMBIT *inst;

	inst = new COMBIT();
	inst->set_bus_config("COMBIT");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("COMBIT",makeCOMBIT);
}
