#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <mixerr.h>
#include <Instrument.h>
#include "AM.h"
#include <rt.h>
#include <rtdefs.h>


extern "C" {
	#include <ugens.h>
	extern int resetval;
}

AM::AM() : Instrument()
{
	in = NULL;
}

AM::~AM()
{
	delete [] in;
}


int AM::init(float p[], short n_args)
{
// p0 = output skip; p1 = input skip; p2 = output duration
// p3 = amplitude multiplier; p4 = AM modulator frequency (hz)
// p5 = input channel [optional]
// p6 = stereo spread <0-1> [optional]
// assumes function table 1 is the amplitude envelope
// assumes function table 2 is the AM modulator waveform

	rtsetinput(p[1], this);
	nsamps = rtsetoutput(p[0], p[2], this);

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(p[2], amplen, amptabs);
	}
	else
		printf("Setting phrase curve to all 1's\n");

	amtable = floc(2);
	if (amtable == NULL) {
		fprintf(stderr, "You need a function table 2 containing mod. waveform\n");
		exit(1);
	}
	lenam = fsize(2);
	si = p[4] * (float)lenam/SR;

	amp = p[3];
	skip = (int)(SR/(float)resetval);      // how often to update amp curve
	phase = 0.0;
	inchan = (int)p[5];
	if ((inchan+1) > inputchans) {
		fprintf(stderr,
			"uh oh, you have asked for channel %d of a %d-channel file...\n",
																				inchan,inputchans);
		exit(-1);
		}

	spread = p[6];

	return(nsamps);
}

int AM::run()
{
	int i,rsamps;
	float out[2];
	float aamp;
	int branch;

	if (in == NULL)        /* first time, so allocate it */
		in = new float [RTBUFSAMPS * inputchans];

	Instrument::run();

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	aamp = amp;            /* in case amptable == NULL */

	branch = 0;
	for (i = 0; i < rsamps; i += inputchans)  {
		if (--branch < 0) {
			if (amptable)
				aamp = tablei(cursamp, amptable, amptabs) * amp;
			branch = skip;
			}

		out[0] = in[i+inchan] * oscili(aamp, si, amtable, lenam, &phase);

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
makeAM()
{
	AM *inst;

	inst = new AM();
	inst->set_bus_config("AM");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("AM",makeAM);
}

