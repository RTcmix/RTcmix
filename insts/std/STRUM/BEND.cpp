#include <iostream.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "BEND.h"
#include <rt.h>
#include <rtdefs.h>

extern strumq *curstrumq[6];

extern "C" {
	void sset(float, float, float, strumq*);
	float strum(float, strumq*);
}

BEND::BEND() : Instrument()
{
	// future setup here?
}

int BEND::init(float p[], int n_args)
{
// p0 = start; p1 = dur; p2 = pitch0 (oct.pc); p3 = pitch1 (oct.pc);
// p4 = gliss function; p5 = fundamental decay time; p6 = nyquist decay time;
// p7 = update every nsamples; p8 = stereo spread [optional]

	float dur;

	dur = p[1];
	nsamps = rtsetoutput(p[0], dur, this);

	strumq1 = curstrumq[0];
	freq0 = cpspch(p[2]);
	freq1 = cpspch(p[3]);
	diff = freq1 - freq0;

	tf0 = p[5];
	tfN = p[6];
	sset(freq0, tf0, tfN, strumq1);

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(dur, amplen, amptabs);
	}
	else
		advise("BEND", "Setting phrase curve to all 1's.");

	glissf = floc((int)p[4]);
	if (glissf) {
		int leng = fsize((int)p[4]);
		tableset(p[1],leng,tags);
	}
	else
		die("BEND", "You haven't made the glissando function (table %d).",
						(int)p[4]);

	reset = (int)p[7];
	if (reset == 0) reset = 100;
	spread = p[8];

	return(nsamps);
}

int BEND::run()
{
	int i;
	float freq;
	float aamp, out[2];
	int branch;

	Instrument::run();

	aamp = 1.0;                  /* in case amptable == NULL */

	branch = 0;
	for (i = 0; i < chunksamps; i++) {
		if (--branch < 0) {
			if (amptable)
				aamp = tablei(cursamp, amptable, amptabs);
			freq = diff * tablei(cursamp, glissf, tags) + freq0;
			sset(freq, tf0, tfN, strumq1);
			branch = reset;
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
makeBEND()
{
	BEND *inst;

	inst = new BEND();
	inst->set_bus_config("BEND");

	return inst;
}
