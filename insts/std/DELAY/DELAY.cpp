#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "DELAY.h"
#include <rt.h>
#include <rtdefs.h>


DELAY::DELAY() : Instrument()
{
    in = NULL;
    delarray = NULL;
}

DELAY::~DELAY()
{
    delete [] in;
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

	long delsamps;

	rtsetinput(p[1], this);
	nsamps = rtsetoutput(p[0], p[2]+p[6], this);
	insamps = (int)(p[2] * SR);

	delsamps = (long)(p[4] * SR + 0.5);
	if( (delarray = new float [delsamps]) == NULL )
		die("DELAY", "Sorry, Charlie -- no space");

	wait = p[4];
	regen = p[5];
	delset(delarray, deltabs, wait);

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(p[2], amplen, amptabs);
	}
	else
		advise("DELAY", "Setting phrase curve to all 1's.");

	amp = p[3];
	skip = (int)(SR/(float)resetval);
	inchan = (int)p[7];
	if ((inchan+1) > inputchans)
		die("DELAY", "You asked for channel %d of a %d-channel file.",
                                                       inchan, inputchans);

	spread = p[8];

	return(nsamps);
}

int DELAY::run()
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

	aamp = amp;           /* in case amptable == NULL */

	branch = 0;
	for (i = 0; i < rsamps; i += inputchans)  {
		if (cursamp > insamps) {
			out[0] = delget(delarray, wait, deltabs) * regen;
			}
		else {
			if (--branch < 0) {
				if (amptable)
					aamp = tablei(cursamp, amptable, amptabs) * amp;
				branch = skip;
				}
			out[0] = (in[i+inchan]*aamp) + (delget(delarray, wait, deltabs)*regen);
			}

		delput(out[0], delarray, deltabs);

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
makeDELAY()
{
	DELAY *inst;

	inst = new DELAY();
	inst->set_bus_config("DELAY");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("DELAY",makeDELAY);
}

