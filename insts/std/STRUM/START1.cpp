#include <ugens.h>
#include <Instrument.h>
#include "START1.h"
#include <rt.h>
#include <rtdefs.h>

extern StrumQueue *curstrumq[6];
DelayQueue *curdelayq;

extern "C" {
	void sset(float, float, float, float, strumq*);
	void randfill(float, int, strumq*);
	float strum(float, strumq*);
	void delayset(float, float, delayq*);
	void delayclean(delayq*);
	float dist(float);
	float delay(float, delayq*);
}

START1::START1() : Instrument()
{
	branch = 0;
}

START1::~START1()
{
// BGGx -- deleting the strumq (and dq) disables the 'carry-over' necessary
// for BEND/FRET/etc. to work.  There has to be a better way, but this is
// an ancient instrument
//	strumq1->unref();
//	dq->unref();
}

int START1::init(double p[], int n_args)
{
// p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
// p4 = nyquist decay time; p5 = distortion gain; p6 = feedback gain
// p7 = feedback pitch (oct.pc); p8 = clean signal level
// p9 = distortion signal level; p10 = amp; p11 = squish
// p12 = stereo spread [optional]
// p13 = flag for deleting pluck arrays (used by FRET, BEND, etc.) [optional]

	float dur = p[1];

	if (rtsetoutput(p[0], dur, this) == -1)
		return DONT_SCHEDULE;
	 
	strumq1 = new StrumQueue;
	strumq1->ref();
	curstrumq[0] = strumq1;
	float freq = cpspch(p[2]);
	sset(SR, freq, p[3], p[4], strumq1);
	randfill(1.0, (int)p[11], strumq1);

	dq = new DelayQueue;
	dq->ref();
	curdelayq = dq;
	delayset(SR, cpspch(p[7]), dq);
	delayclean(dq);

	dgain = p[5];
	fbgain = p[6]/dgain;
	cleanlevel = p[8];
	distlevel = p[9];
	amp = p[10];
	spread = p[12];
	deleteflag = (int)p[13];

	d = 0.0;

   amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, dur, amplen, amptabs);
	}
	else {
		rtcmix_advise("START1", "Setting phrase curve to all 1's.");
		aamp = amp;
	}

	skip = (int)(SR / (float)resetval);

	return nSamps();
}

int START1::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			if (amptable)
				aamp = tablei(currentFrame(), amptable, amptabs) * amp;
			branch = skip;
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
makeSTART1()
{
	START1 *inst;

	inst = new START1();
	inst->set_bus_config("START1");

	return inst;
}
