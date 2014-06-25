#include <ugens.h>
#include <Instrument.h>
#include "BEND1.h"
#include <rt.h>
#include <rtdefs.h>

extern StrumQueue *curstrumq[6];
extern DelayQueue *curdelayq;

extern "C" {
	void sset(float, float, float, float, strumq*);
	float strum(float, strumq*);
	void delayset(float, float, delayq*);
	float dist(float);
	float delay(float, delayq*);
}

BEND1::BEND1() : Instrument()
{
	branch = 0;
}

BEND1::~BEND1()
{
// BGGx -- deleting the strumq disables the 'carry-over' necessary for
// BEND/FRET/etc. to work.  There has to be a better way, but this is
// an ancient instrument
//	strumq1->unref();
}

int BEND1::init(double p[], int n_args)
{
// p0 = start; p1 = dur; p2 = pitch0 (oct.pc); p3 = pitch1 (oct.pc)
// p4 = gliss function #; p5 = fundamental decay time
// p6 = nyquist decay time; p7 = distortion gain; p8 = feedback gain
// p9 = feedback pitch (oct.pc); p10 = clean signal level
// p11 = distortion signal level; p12 = amp; p13 = update gliss nsamples
// p14 = stereo spread [optional]

	float dur = p[1];

	if (rtsetoutput(p[0], dur, this) == -1)
		return DONT_SCHEDULE;

	strumq1 = curstrumq[0];
	strumq1->ref();
	freq0 = cpspch(p[2]);
	diff = cpspch(p[3]) - freq0;
	tf0 = p[5];
	tfN = p[6];
	sset(SR, freq0, tf0, tfN, strumq1);

	dq = curdelayq;
	delayset(SR, cpspch(p[9]), dq);

	amp = p[12];
	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, dur, amplen, amptabs);
	}
	else {
		rtcmix_advise("BEND1", "Setting phrase curve to all 1's.");
		aamp = amp;
	}

	glissf = floc((int)p[4]);
	if (glissf) {
		int leng = fsize((int)p[4]);
		tableset(SR, p[1],leng,tags);
	}
	else
		return die("BEND1", "You haven't made the glissando function (table %d).",
						(int)p[4]);

	dgain = p[7];
	fbgain = p[8]/dgain;
	cleanlevel = p[10];
	distlevel = p[11];
	reset = (int)p[13];
	if (reset == 0) reset = 100;
	spread = p[14];

	d = 0.0;

	return nSamps();
}

int BEND1::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			if (amptable)
				aamp = tablei(currentFrame(), amptable, amptabs) * amp;
			float freq = diff * tablei(currentFrame(), glissf, tags) + freq0;
			sset(SR, freq, tf0, tfN, strumq1);
			branch = reset;
		}

		float a = strum(d, strumq1);
		float b = dist(dgain*a);
		d = fbgain*delay(b, dq);

		float out[2];
		out[0] = (cleanlevel*a + distlevel*b) * aamp;

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
makeBEND1()
{
	BEND1 *inst;

	inst = new BEND1();
	inst->set_bus_config("BEND1");

	return inst;
}
