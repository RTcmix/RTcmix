#include <iostream.h>
#include <unistd.h>
#include <stdio.h>
#include <mixerr.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>
#include "MIX.h"

extern "C" {
	#include <ugens.h>
	extern int resetval;
}


MIX::MIX() : Instrument()
{
}

MIX::~MIX()
{
}

int MIX::init(float p[], short n_args)
{
// p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; p4-n = channel mix matrix
// we're stashing the setline info in gen table 1

	int i;

	if (p[2] < 0.0) p[2] = -p[2] - p[1];

	nsamps = rtsetoutput(p[0], p[2], this);
	rtsetinput(p[1], this);

	amp = p[3];

	for (i = 0; i < inputchans; i++) {
		outchan[i] = (int)p[i+4];
		if (outchan[i] + 1 > outputchans) {
			fprintf(stderr, "You wanted output channel %d, but have only "
									"specified %d output channels\n",
									outchan[i], outputchans);
			exit(-1);
			}
		}

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(p[2], amplen, tabs);
	}
	else
		printf("Setting phrase curve to all 1's\n");

	skip = (int)(SR/(float)resetval); // how often to update amp curve, default 200/sec.

	return(nsamps);
}

int MIX::run()
{
	int i,j,k,rsamps;
	float in[MAXBUF], out[MAXBUS];
	float aamp;
	int branch;

	Instrument::run();

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	branch = 0;
	for (i = 0; i < rsamps; i += inputchans)  {
		if (--branch < 0) {
			if (amptable)
				aamp = tablei(cursamp, amptable, tabs) * amp;
			else
				aamp = amp;
			branch = skip;
			}

		for (j = 0; j < outputchans; j++) {
			out[j] = 0.0;
			for (k = 0; k < inputchans; k++) {
				if (outchan[k] == j)
					out[j] += in[i+k] * aamp;
				}
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
	inst->set_bus_config("MIX");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("MIX",makeMIX);
}


