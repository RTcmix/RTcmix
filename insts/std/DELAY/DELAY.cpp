#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <mixerr.h>
#include <Instrument.h>
#include "DELAY.h"
#include <rt.h>
#include <rtdefs.h>

extern "C" {
	#include <ugens.h>
	extern int resetval;
}

DELAY::DELAY() : Instrument()
{
    delarray = NULL;
}

DELAY::~DELAY()
{
    delete [] delarray;
}

int DELAY::init(float p[], short n_args)
{
// p0 = output skip; p1 = input skip; p2 = output duration
// p3 = amplitude multiplier; p4 = delay time
// p5 = regeneration multiplier (< 1!)
// p6 = ring-down duration
// p7 = input channel [optional]
// p8 = stereo spread (0-1) <optional>
// assumes function table 1 is the amplitude envelope

	int amplen;
	long delsamps;

	rtsetinput(p[1], this);
	nsamps = rtsetoutput(p[0], p[2]+p[6], this);
	insamps = p[2] * SR;

	delsamps = p[4] * SR + 0.5;
	if( (delarray = new float [delsamps]) == NULL ) {
		fprintf(stderr,"Sorry, Charlie -- no space\n");
		exit(-1);
		}
	wait = p[4];
	regen = p[5];
	delset(delarray, deltabs, wait);

	amptable = floc(1);
	amplen = fsize(1);
	tableset(p[2], amplen, amptabs);

	amp = p[3];
	skip = SR/(float)resetval;
	inchan = p[7];
	if ((inchan+1) > inputchans) {
		fprintf(stderr,"uh oh, you have asked for channel %d of a %d-channel file...\n",inchan,inputchans);
		exit(-1);
		}

	spread = p[8];

	return(nsamps);
}

int DELAY::run()
{
	int i,rsamps;
	float in[2*MAXBUF],out[2];
	float aamp;
	int branch;

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	branch = 0;
	for (i = 0; i < rsamps; i += inputchans)  {
		if (cursamp > insamps) {
			out[0] = delget(delarray, wait, deltabs) * regen;
			}
		else {
			if (--branch < 0) {
				aamp = tablei(cursamp, amptable, amptabs) * amp;
				branch = skip;
				}
			out[0] = (in[i+inchan]*aamp) + (delget(delarray, wait, deltabs)*regen);
			}

		delput(out[0], delarray, deltabs);

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
makeDELAY()
{
	DELAY *inst;

	inst = new DELAY();
	return inst;
}

void
rtprofile()
{
	RT_INTRO("DELAY",makeDELAY);
}

