#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "SCULPT.h"
#include <rt.h>
#include <rtdefs.h>


SCULPT::SCULPT() : Instrument()
{
	// future setup here?
}

int SCULPT::init(double p[], int n_args)
{
// p0 = start; p1 = point dur; p2 = overall amplitude; p3 = number of points
// p4 = stereo spread [optional];
// function slot 2 is waveform, slot 1 is overall amp envelope
// function slot 3 is frequency points, slot 4 is amplitude points

	float tdur;

	nsamps = rtsetoutput(p[0], p[1]*p[3], this);
	tdur = p[1] * p[3];
	pdur = (int)(p[1] * SR);

	wave = floc(2);
	if (wave == NULL) {
		die("SCULPT", "You need to store a waveform in function 2.");
		return(DONT_SCHEDULE);
	}
	len = fsize(2);

	amptable = floc(1);
	if (amptable) {
		int len = fsize(1);
		tableset(SR, tdur, len, amptabs);
	}
	else
		advise("SCULPT", "Setting phrase curve to all 1's.");

	freqtable = floc(3);
	if (freqtable == NULL) {
		die("SCULPT",
			"You haven't made the table of frequency points (table 3).");
		return(DONT_SCHEDULE);
	}

	pamptable = floc(4);
	if (pamptable == NULL) {
		die("SCULPT",
			"You haven't made the table of amplitude points (table 4).");
		return(DONT_SCHEDULE);
	}

	amp = p[2];
	phase = 0.0;
	pcount = 0;
	index = 0;

	spread = p[4];

	return(nsamps);
}

int SCULPT::run()
{
	int i;
	float out[2];
	float si=0.;
	float overamp, aamp=0.;

	overamp = amp;            /* in case amptable == NULL */

	for (i = 0; i < chunksamps; i++) {
		if (--pcount < 0) {
			si = freqtable[index] * (float)len/SR;
			if (amptable)
				overamp = table(cursamp, amptable, amptabs) * amp;
			aamp = ampdb(60.0 + pamptable[index]) * overamp;
			index++;
			pcount = pdur;
			}
		out[0] = oscil(aamp, si, wave, len, &phase);

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
makeSCULPT()
{
	SCULPT *inst;

	inst = new SCULPT();
	inst->set_bus_config("SCULPT");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("SCULPT",makeSCULPT);
}

