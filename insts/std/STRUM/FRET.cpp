#include <iostream.h>
#include <mixerr.h>
#include <Instrument.h>
#include "FRET.h"
#include <rt.h>
#include <rtdefs.h>

extern strumq *curstrumq[6];

extern "C" {
	#include <ugens.h>
	void sset(float, float, float, strumq*);
	float strum(float, strumq*);
}

FRET::FRET() : Instrument()
{
	// future setup here?
}

int FRET::init(float p[], short n_args)
{
// p0 = start; p1 = dur; p2 = pitch(oct.pc); p3 = fundamental decay time;
// p4 = nyquist decay time; p5 = stereo spread [optional]

	nsamps = rtsetoutput(p[0], p[1], this);

	strumq1 = curstrumq[0];
	freq = cpspch(p[2]);
	tf0 = p[3];
	tfN = p[4];

	spread = p[5];
	firsttime = 1;

	return(nsamps);
}

int FRET::run()
{
	int i;
	float out[2];

	Instrument::run();

	if (firsttime) {
		sset(freq, tf0, tfN, strumq1);
		firsttime = 0;
		}

	for (i = 0; i < chunksamps; i++) {
		out[0] = strum(0.,strumq1);

		if (NCHANS == 2) { /* split stereo files between the channels */
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
