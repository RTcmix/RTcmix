#include <stdlib.h>
#include <stdio.h>
#include <ugens.h>
#include <Instrument.h>
#include "SCULPT.h"
#include <rt.h>
#include <rtdefs.h>


SCULPT::SCULPT() : Instrument()
{
	branch = 0;
}

int SCULPT::init(double p[], int n_args)
{
// p0 = start; p1 = point dur; p2 = overall amplitude; p3 = number of points
// p4 = stereo spread [optional];
// function slot 2 is waveform, slot 1 is overall amp envelope
// function slot 3 is frequency points, slot 4 is amplitude points

	nsamps = rtsetoutput(p[0], p[1]*p[3], this);
	float tdur = p[1] * p[3];
	pdur = (int)(p[1] * SR);

	wave = floc(2);
	if (wave == NULL)
		return die("SCULPT", "You need to store a waveform in function 2.");
	len = fsize(2);

	amptable = floc(1);
	if (amptable) {
		int len = fsize(1);
		tableset(SR, tdur, len, amptabs);
	}
	else
		advise("SCULPT", "Setting phrase curve to all 1's.");

	freqtable = floc(3);
	if (freqtable == NULL)
		return die("SCULPT",
			"You haven't made the table of frequency points (table 3).");

	pamptable = floc(4);
	if (pamptable == NULL)
		return die("SCULPT",
			"You haven't made the table of amplitude points (table 4).");

	amp = p[2];
	phase = 0.0;
	si = 0.0;
	index = 0;

	spread = p[4];

	return nSamps();
}

int SCULPT::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			si = freqtable[index] * (float)len/SR;
			float overamp;
			if (amptable)
				overamp = table(cursamp, amptable, amptabs) * amp;
			else
				overamp = amp;
			aamp = ampdb(60.0 + pamptable[index]) * overamp;
			index++;
			branch = pdur;
		}

		float out[2];
		out[0] = oscil(aamp, si, wave, len, &phase);

		if (outputChannels() == 2) { /* split stereo files between the channels */
			out[1] = (1.0 - spread) * out[0];
			out[0] *= spread;
		}

		rtaddout(out);
		increment();
	}
	return framesToRun();
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

