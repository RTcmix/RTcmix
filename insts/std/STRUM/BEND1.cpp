#include <iostream.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "BEND1.h"
#include <rt.h>
#include <rtdefs.h>

extern strumq *curstrumq[6];
extern delayq *curdelayq;

extern "C" {
	void sset(float, float, float, strumq*);
	float strum(float, strumq*);
	void delayset(float, delayq*);
	float dist(float);
	float delay(float, delayq*);
}

BEND1::BEND1() : Instrument()
{
	branch = 0;
}

int BEND1::init(float p[], int n_args)
{
// p0 = start; p1 = dur; p2 = pitch0 (oct.pc); p3 = pitch1 (oct.pc)
// p4 = gliss function #; p5 = fundamental decay time
// p6 = nyquist decay time; p7 = distortion gain; p8 = feedback gain
// p9 = feedback pitch (oct.pc); p10 = clean signal level
// p11 = distortion signal level; p12 = amp; p13 = update gliss nsamples
// p14 = stereo spread [optional]

	float dur = p[1];

	nsamps = rtsetoutput(p[0], dur, this);

	strumq1 = curstrumq[0];
	freq0 = cpspch(p[2]);
	diff = cpspch(p[3]) - freq0;
	tf0 = p[5];
	tfN = p[6];
	sset(freq0, tf0, tfN, strumq1);

	dq = curdelayq;
	delayset(cpspch(p[9]), dq);

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(dur, amplen, amptabs);
	}
	else {
		advise("FRET", "Setting phrase curve to all 1's.");
		aamp = amp;
	}

	glissf = floc((int)p[4]);
	if (glissf) {
		int leng = fsize((int)p[4]);
		tableset(p[1],leng,tags);
	}
	else
		die("BEND1", "You haven't made the glissando function (table %d).",
						(int)p[4]);

	dgain = p[7];
	fbgain = p[8]/dgain;
	cleanlevel = p[10];
	distlevel = p[11];
	amp = p[12];
	reset = (int)p[13];
	if (reset == 0) reset = 100;
	spread = p[14];

	d = 0.0;

	return(nsamps);
}

int BEND1::run()
{
	int i;
	float out[2];
	float freq;
	float a,b;

	Instrument::run();

	for (i = 0; i < chunksamps; i++) {
		if (--branch < 0) {
			if (amptable)
				aamp = tablei(cursamp, amptable, amptabs) * amp;
			freq = diff * tablei(cursamp, glissf, tags) + freq0;
			sset(freq, tf0, tfN, strumq1);
			branch = reset;
			}

		a = strum(d, strumq1);
		b = dist(dgain*a);
		d = fbgain*delay(b, dq);

		out[0] = (cleanlevel*a + distlevel*b) * aamp;

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
makeBEND1()
{
	BEND1 *inst;

	inst = new BEND1();
	inst->set_bus_config("BEND1");

	return inst;
}
