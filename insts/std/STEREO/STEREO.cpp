#include <iostream.h>
#include <unistd.h>
#include <stdio.h>
#include <mixerr.h>
#include <Instrument.h>
#include "STEREO.h"
#include <rt.h>
#include <rtdefs.h>


extern "C" {
	#include <ugens.h>
	extern int resetval;
}

STEREO::STEREO() : Instrument()
{
	// future setup here?
}

int STEREO::init(float p[], short n_args)
{
// p0 = outsk; p1 = insk; p2 = dur (-endtime); p3 = amp; p4-n = channel mix matrix
// we're stashing the setline info in gen table 1

	int i;

	if (NCHANS != 2) {
		fprintf(stderr,"output must be stereo!\n");
		exit(-1);
		}

	if (p[2] < 0.0) p[2] = -p[2] - p[1];

	nsamps = rtsetoutput(p[0], p[2], this);
	rtsetinput(p[1], this);

	amp = p[3];

	for (i = 0; i < inputchans; i++) {
		outspread[i] = p[i+4];
		}

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(p[2], amplen, tabs);
	}
	else
		printf("Setting phrase curve to all 1's\n");

	skip = (int)(SR/(float)resetval);       // how often to update amp curve

	return(nsamps);
}

int STEREO::run()
{
	int i,j,rsamps;
	float in[2*MAXBUF],out[2];
	float aamp;
	int branch;

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	aamp = amp;        /* in case amptable == NULL */

	branch = 0;
	for (i = 0; i < rsamps; i += inputchans)  {
		if (--branch < 0) {
			if (amptable)
				aamp = table(cursamp, amptable, tabs) * amp;
			branch = skip;
			}

		out[0] = out[1] = 0.0;
		for (j = 0; j < inputchans; j++) {
			if (outspread[j] >= 0.0) {
				out[0] += in[i+j] * outspread[j] * aamp;
				out[1] += in[i+j] * (1.0 - outspread[j]) * aamp;
				}
			}

		rtaddout(out);
		cursamp++;
		}
	return i;
}



Instrument*
makeSTEREO()
{
	STEREO *inst;

	inst = new STEREO();
	return inst;
}

void
rtprofile()
{
	RT_INTRO("STEREO",makeSTEREO);
}

