#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../sys/mixerr.h"
#include "../../rtstuff/Instrument.h"
#include "COMBIT.h"
#include "../../rtstuff/rt.h"
#include "../../rtstuff/rtdefs.h"


extern int lineset;
extern int resetval;

extern "C" {
#include "../../H/ugens.h"
#include "../../H/combs.h"
}

COMBIT::COMBIT() : Instrument()
{
    combarr = NULL;
}

COMBIT::~COMBIT()
{
    delete [] combarr;
}

int COMBIT::init(float p[], short n_args)
{
// p0 = outsk; p1 = insk; p2 = input dur; p3 = smplitude multiplier
// p4 = pitch (cps); p5 = reverb time; p6 = input channel [optional]
// p7 = stereo spread [optional]
// uses setline for amp envelope, filling gen slot 1 internally

	float loopt;
	int amplen;

	rtsetinput(p[1], this);
	nsamps = rtsetoutput(p[0], p[2]+p[5], this);
	insamps = p[2] * SR;

	loopt = 1.0/p[4];

	// adding "START" didn't do it, but adding 10.0 seems
	// to prevent the array out-of-bounds bug
	// BGG
	combarr = new float[int(loopt * SR + 10.0)];

	if (!combarr) {
	    fprintf(stderr, "could not allocate memory for comb array!\n");
	    return (-1);
	}
	combset(loopt,p[5],0,combarr);

	if (lineset) {
		amptable = floc(1);
		amplen = fsize(1);
		tableset(p[2]+p[5], amplen, tabs);
		}
	amp = p[3];
	skip = SR/(float)resetval; // how often to update amp curve, default 200/sec
	inchan = p[6];
	if ((inchan+1) > inputchans) {
		fprintf(stderr,"uh oh, you have asked for channel %d of a %d-channel file...\n",inchan,inputchans);
		exit(-1);
		}

	spread = p[7];

	return(nsamps);
}

int COMBIT::run()
{
	int i,j,rsamps;
	float in[2*MAXBUF],out[2];
	float aamp;
	int branch;

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	branch = 0;
	for (i = 0; i < rsamps; i += inputchans)  {
		if (cursamp > insamps) {
			for (j = 0; j < inputchans; j++) in[i+j] = 0.0;
			}

		if (--branch < 0) {
			if (lineset) aamp = table(cursamp, amptable, tabs) * amp;
			else aamp = amp;
			branch = skip;
			}

		out[0] = comb(in[i+inchan],combarr) * aamp;
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
makeCOMBIT()
{
	COMBIT *inst;

	inst = new COMBIT();
	return inst;
}

void
rtprofile()
{
	RT_INTRO("COMBIT",makeCOMBIT);
}

