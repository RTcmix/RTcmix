#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../sys/mixerr.h"
#include "../../rtstuff/Instrument.h"
#include "AM.h"
#include "../../rtstuff/rt.h"
#include "../../rtstuff/rtdefs.h"


extern "C" {
	#include "../../H/ugens.h"
	extern int resetval;
}

AM::AM() : Instrument()
{
	// future setup here?
}

int AM::init(float p[], short n_args)
{
// p0 = output skip; p1 = input skip; p2 = output duration
// p3 = amplitude multiplier; p4 = AM modulator frequency (hz)
// p5 = input channel [optional]
// p6 = stereo spread <0-1> [optional]
// assumes function table 1 is the amplitude envelope
// assumes function table 2 is the AM modulator waveform

	int amplen;

	rtsetinput(p[1], this);
	nsamps = rtsetoutput(p[0], p[2], this);

	amptable = floc(1);
	amplen = fsize(1);
	tableset(p[2], amplen, amptabs);

	amtable = floc(2);
	lenam = fsize(2);
	si = p[4] * (float)lenam/SR;

	amp = p[3];
	skip = SR/(float)resetval; // how often to update amp curve, default 200/sec
	phase = 0.0;
	inchan = p[5];
	if ((inchan+1) > inputchans) {
		fprintf(stderr,"uh oh, you have asked for channel %d of a %d-channel file...\n",inchan,inputchans);
		exit(-1);
		}

	spread = p[6];

	return(nsamps);
}

int AM::run()
{
	int i,rsamps;
	float in[2*MAXBUF],out[2];
	float aamp;
	int branch;

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	branch = 0;
	for (i = 0; i < rsamps; i += inputchans)  {
		if (--branch < 0) {
			aamp = tablei(cursamp, amptable, amptabs) * amp;
			branch = skip;
			}

		out[0] = in[i+inchan] * oscili(aamp, si, amtable, lenam, &phase);

		if (NCHANS == 2) {
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
	return inst;
}

void
rtprofile()
{
	RT_INTRO("AM",makeAM);
}

