#include <iostream.h>
#include <unistd.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>
#include "MIX.h"


MIX::MIX() : Instrument(), in(NULL)
{
	branch = 0;
}

MIX::~MIX()
{
	delete [] in;
}

int MIX::init(double p[], int n_args)
{
// p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; p4-n = channel mix matrix
// we're stashing the setline info in gen table 1

	int i, rvin;

	if (p[2] < 0.0) p[2] = -p[2] - p[1];

	nsamps = rtsetoutput(p[0], p[2], this);
	rvin = rtsetinput(p[1], this);
	if (rvin == -1) { // no input
		return(DONT_SCHEDULE);
	}

	amp = p[3];

	for (i = 0; i < inputchans; i++) {
		outchan[i] = (int)p[i+4];
		if (outchan[i] + 1 > outputchans) {
			die("MIX", "You wanted output channel %d, but have only specified "
							"%d output channels", outchan[i], outputchans);
			return(DONT_SCHEDULE);
		}
	}

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(p[2], amplen, tabs);
	}
	else
		advise("MIX", "Setting phrase curve to all 1's.");
   aamp = amp;                     /* in case amptable == NULL */

	skip = (int)(SR/(float)resetval);

	return(nsamps);
}

int MIX::configure()
{
	in = new float [RTBUFSAMPS * inputchans];
	return in ? 0 : -1;
}

int MIX::run()
{
	int i,j,k,rsamps;
	float out[MAXBUS];

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	for (i = 0; i < rsamps; i += inputchans)  {
		if (--branch < 0) {
			if (amptable)
				aamp = tablei(cursamp, amptable, tabs) * amp;
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
