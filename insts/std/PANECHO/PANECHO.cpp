#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "PANECHO.h"
#include <rt.h>
#include <rtdefs.h>


PANECHO::PANECHO() : Instrument()
{
	in = NULL;
}

PANECHO::~PANECHO()
{
	delete [] in;
	delete [] delarray1;
	delete [] delarray2;
}

int PANECHO::init(float p[], int n_args)
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
	int rvin;

	rvin = rtsetinput(p[1], this);
	if (rvin == -1) { // no input
		return(DONT_SCHEDULE);
	}
	nsamps = rtsetoutput(p[0], p[2]+p[7], this);
	insamps = (int)(p[2] * SR);

	if (outputchans != 2) {
		die("PANECHO", "Output must be stereo.");
		return(DONT_SCHEDULE);
	}

	delsamps = (long)(p[4] * SR + 0.5);
	delarray1 = new float[delsamps];
	if (!delarray1) {
		die("PANECHO", "Sorry, Charlie -- no space");
		return(DONT_SCHEDULE);
	}

	wait1 = p[4];
	delset(delarray1, deltabs1, wait1);

	delsamps = (long)(p[5] * SR + 0.5);
	delarray2 = new float[delsamps];
	if (!delarray2) {
		die("PANECHO", "Sorry, Charlie -- no space");
		return(DONT_SCHEDULE);
	}

	wait2 = p[5];
	delset(delarray2, deltabs2, wait2);

	regen = p[6];

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(p[2], amplen, amptabs);
	}
	else
		advise("PANECHO", "Setting phrase curve to all 1's.");

	amp = p[3];
	skip = (int)(SR/(float)resetval);
	inchan = (int)p[8];
	if ((inchan+1) > inputchans) {
		die("PANECHO", "You asked for channel %d of a %d-channel file.",
                                                       inchan, inputchans);
		return(DONT_SCHEDULE);
	}

	return(nsamps);
}

int PANECHO::run()
{
	int i,rsamps;
	float out[2];
	float aamp;
	int branch;

	if (in == NULL)     /* first time, so allocate it */
		in = new float [RTBUFSAMPS * inputchans];

	Instrument::run();

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
	inst->set_bus_config("PANECHO");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("PANECHO",makePANECHO);
}

