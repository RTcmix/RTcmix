#include <iostream.h>
#include <unistd.h>
#include <stdio.h>
#include "../../sys/mixerr.h"
#include "../../rtstuff/Instrument.h"
#include "../../rtstuff/rt.h"
#include "../../rtstuff/rtdefs.h"
#include "MIX.h"


extern "C" {
	#include "../../H/ugens.h"
	extern int lineset;
	extern int resetval;
}

MIX::MIX() : Instrument()
{
	// future setup here?
}

int MIX::init(float p[], short n_args)
{
// p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; p4-n = channel mix matrix
// we're stashing the setline info in gen table 1

	int i, amplen;

	if (p[2] < 0.0) p[2] = -p[2] - p[1];

	nsamps = rtsetoutput(p[0], p[2], this);
	rtsetinput(p[1], this);

	amp = p[3];

	for (i = 0; i < inputchans; i++) {
		outchan[i] = p[i+4];
		if (outchan[i]+1 > NCHANS) {
			fprintf(stderr,"You wanted output channel %d, but have only specified %d output channels\n",outchan[i],NCHANS);
			exit(-1);
			}
		}

	if (lineset) {
		amptable = floc(1);
		amplen = fsize(1);
		tableset(p[2], amplen, tabs);
		}

	skip = SR/(float)resetval; // how often to update amp curve, default 200/sec.

	return(nsamps);
}

int MIX::run()
{
	int i,j,rsamps;
	float in[2*MAXBUF],out[2];
	float aamp;
	int branch;

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	branch = 0;
	for (i = 0; i < rsamps; i += inputchans)  {
		if (--branch < 0) {
			if (lineset) aamp = table(cursamp, amptable, tabs) * amp;
			else aamp = amp;
			branch = skip;
			}

		for (j = 0; j < NCHANS; j++) {
			out[j] = 0.0;
			if (outchan[j] == j) out[j] = in[i+outchan[j]] * aamp;
			}

		rtaddout(out);
		cursamp++;
		}
	return i;
}



Instrument*
makeMIX()
{
	MIX *inst;

	inst = new MIX();
	return inst;
}

void
rtprofile()
{
	RT_INTRO("MIX",makeMIX);
}


