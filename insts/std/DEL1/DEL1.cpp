#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <mixerr.h>
#include <Instrument.h>
#include "DEL1.h"
#include <rt.h>
#include <rtdefs.h>

extern "C" {
	#include <ugens.h>
	extern int resetval;
}

DEL1::DEL1() : Instrument()
{
    in = NULL;
    delarray = NULL;
}

DEL1::~DEL1()
{
    delete [] in;
    delete [] delarray;
}

int DEL1::init(float p[], short n_args)
{
// p0 = output skip; p1 = input skip; p2 = output duration
// p3 = amplitude multiplier; p4 = delay time
// p5 = delay amplitude multiplier
// p6 = input channel [optional]
// assumes function table 1 is the amplitude envelope

	long delsamps;

	rtsetinput(p[1], this);
	nsamps = rtsetoutput(p[0], p[2]+p[4], this);
	insamps = (int)(p[2] * SR);

	if (outputchans != 2) {
		fprintf(stderr,"output must be stereo!!!!!!!\n");
		return(-1);
		}

	delsamps = (long)(p[4]*SR + 0.5);
	if( (delarray = new float[delsamps]) == NULL ) {
		fprintf(stderr,"Sorry, Charlie -- no space\n");
		exit(-1);
		}
	wait = p[4];
	delamp = p[5];
	delset(delarray,deltabs,wait);

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(p[2], amplen, amptabs);
	}
	else
		printf("Setting phrase curve to all 1's\n");

	amp = p[3];
	skip = (int)(SR/(float)resetval);
	inchan = (int)p[6];
	if ((inchan+1) > inputchans) {
		fprintf(stderr,"uh oh, you have asked for channel %d of a %d-channel file...\n",inchan,inputchans);
		exit(-1);
		}

	return(nsamps);
}

int DEL1::run()
{
	int i,j,rsamps;
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
			for (j = 0; j < inputchans; j++) in[i+j] = 0.0;
			}

		if (--branch < 0) {
			if (amptable)
				aamp = tablei(cursamp, amptable, amptabs) * amp;
			branch = skip;
			}

		in[i+inchan] *= aamp;
		out[0] = in[i+inchan];
		out[1] = delget(delarray, wait, deltabs) * delamp;
		rtaddout(out);
		delput(in[i+inchan], delarray, deltabs);
		cursamp++;
		}
	return(i);
}



Instrument*
makeDEL1()
{
	DEL1 *inst;

	inst = new DEL1();
	inst->set_bus_config("DEL1");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("DEL1",makeDEL1);
}

