#include <iostream.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "FRET.h"
#include <rt.h>
#include <rtdefs.h>

extern strumq *curstrumq[6];

extern "C" {
	void sset(float, float, float, strumq*);
	float strum(float, strumq*);
}

FRET::FRET() : Instrument()
{
	branch = 0;
}

int FRET::init(double p[], int n_args)
{
// p0 = start; p1 = dur; p2 = pitch(oct.pc); p3 = fundamental decay time;
// p4 = nyquist decay time; p5 = stereo spread [optional]

	float	dur = p[1];

	nsamps = rtsetoutput(p[0], p[1], this);

	strumq1 = curstrumq[0];
	freq = cpspch(p[2]);
	tf0 = p[3];
	tfN = p[4];

	spread = p[5];
	firsttime = 1;

   amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(dur, amplen, amptabs);
	}
	else {
		advise("FRET", "Setting phrase curve to all 1's.");
		aamp = 1.0;
	}

	skip = (int)(SR / (float)resetval);

	return(nsamps);
}

int FRET::run()
{
	int i;
	float out[2];

	if (firsttime) {
		sset(freq, tf0, tfN, strumq1);
		firsttime = 0;
		}

	for (i = 0; i < chunksamps; i++) {
		if (--branch < 0) {
			if (amptable)
				aamp = tablei(cursamp, amptable, amptabs);
			branch = skip;
		}

		out[0] = strum(0.,strumq1) * aamp;

		if (outputchans == 2) { /* split stereo files between the channels */
			out[1] = (1.0 - spread) * out[0];
			out[0] *= spread;
			}

		rtaddout(out);
		cursamp++;
	}
	return i;
}



Instrument*
makeFRET()
{
	FRET *inst;

	inst = new FRET();
	inst->set_bus_config("FRET");

	return inst;
}
