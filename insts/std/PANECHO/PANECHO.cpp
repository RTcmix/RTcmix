#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <mixerr.h>
#include <Instrument.h>
#include "PANECHO.h"
#include <rt.h>
#include <rtdefs.h>

extern "C" {
	#include <ugens.h>
	extern int resetval;
}

PANECHO::PANECHO() : Instrument()
{
	// future setup here?
}

PANECHO::~PANECHO()
{
	delete [] delarray1;
	delete [] delarray2;
}

int PANECHO::init(float p[], short n_args)
{
// p0 = output skip; p1 = input skip; p2 = output duration
// p3 = amplitude multiplier 
// p4 = chennel 0 delay time
// p5 = chennel 1 delay time
// p6 = regeneration multiplier (< 1!)
// p7 = ring-down duration
// p8 = input channel [optional]
// assumes function table 1 is the amplitude envelope

	long delsamps;

	rtsetinput(p[1], this);
	nsamps = rtsetoutput(p[0], p[2]+p[7], this);
	insamps = (int)(p[2] * SR);

	if (NCHANS != 2) {
		fprintf(stderr,"output must be stereo!\n");
		exit(-1);
		}

	delsamps = (long)(p[4] * SR + 0.5);
	delarray1 = new float[delsamps];
	if (!delarray1) {
		fprintf(stderr,"Sorry, Charlie -- no space\n");
		exit(-1);
	}
	wait1 = p[4];
	delset(delarray1, deltabs1, wait1);

	delsamps = (long)(p[5] * SR + 0.5);
	delarray2 = new float[delsamps];
	if (!delarray2) {
		fprintf(stderr,"Sorry, Charlie -- no space\n");
		exit(-1);
	}
	wait2 = p[5];
	delset(delarray2, deltabs2, wait2);

	regen = p[5];

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(p[2], amplen, amptabs);
	}
	else
		printf("Setting phrase curve to all 1's\n");

	amp = p[3];
	skip = (int)(SR/(float)resetval);
	inchan = (int)p[8];
	if ((inchan+1) > inputchans) {
		fprintf(stderr,"uh oh, you have asked for channel %d of a %d-channel file...\n",inchan,inputchans);
		exit(-1);
		}

	return(nsamps);
}

int PANECHO::run()
{
	int i,rsamps;
	float in[2*MAXBUF],out[2];
	float aamp;
	int branch;

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	aamp = amp;         /* in case amptable == NULL */

	branch = 0;
	for (i = 0; i < rsamps; i += inputchans)  {
		if (cursamp > insamps) {
			out[0] = delget(delarray2, wait2, deltabs2) * regen;
			out[1] = delget(delarray1, wait1, deltabs1);
			}
		else {
			if (--branch < 0) {
				if (amptable)
					aamp = tablei(cursamp, amptable, amptabs) * amp;
				branch = skip;
				}
			out[0] = (in[i+inchan]*aamp) + (delget(delarray2, wait2, deltabs2)*regen);
			out[1] = delget(delarray1, wait1, deltabs1);
			}

		delput(out[0], delarray1, deltabs1);
		delput(out[1], delarray2, deltabs2);

		rtaddout(out);
		cursamp++;
		}
	return(i);
}



Instrument*
makePANECHO()
{
	PANECHO *inst;

	inst = new PANECHO();
	return inst;
}

void
rtprofile()
{
	RT_INTRO("PANECHO",makePANECHO);
}

