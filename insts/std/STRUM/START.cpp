#include <stdlib.h>
#include <stdio.h>
#include <ugens.h>
#include <Instrument.h>
#include "START.h"
#include <rt.h>
#include <rtdefs.h>

StrumQueue *curstrumq[6];

extern "C" {
	void sset(float, float, float, float, strumq*);
	void randfill(float, int, strumq*);
	float strum(float, strumq*);
}

START::START() : Instrument()
{
	branch = 0;
}

START::~START()
{
// BGGx -- deleting the strumq disables the 'carry-over' necessary for
// BEND/FRET/etc. to work.  There has to be a better way, but this is
// an ancient instrument
//	strumq1->unref();
}

// p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
// p4 = nyquist decay time; p5 = amp, p6 = squish; p7 = stereo spread [optional]
// p8 = flag for deleting pluck arrays (used by FRET, BEND, etc.) [optional]

int START::init(double p[], int n_args)
{
	float outskip = p[0];
	float dur = p[1];
	float pitch = p[2];
	float fdecay = p[3];
	float nydecay = p[4];
	float amp = p[5];
	int squish = (int)p[6];
	spread = p[7];
	deleteflag = (int)p[8];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	strumq1 = new StrumQueue;
	strumq1->ref();
	curstrumq[0] = strumq1;
	float freq = cpspch(pitch);
	sset(SR, freq, fdecay, nydecay, strumq1);
	randfill(amp, squish, strumq1);

   amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, dur, amplen, amptabs);
	}
	else {
		rtcmix_advise("START", "Setting phrase curve to all 1's.");
		aamp = 1.0;
	}

	skip = (int)(SR / (float)resetval);

	return nSamps();
}

int START::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			if (amptable)
				aamp = tablei(currentFrame(), amptable, amptabs);
			branch = skip;
		}

		float out[2];
		out[0] = strum(0.,strumq1) * aamp;

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
makeSTART()
{
	START *inst;

	inst = new START();
	inst->set_bus_config("START");

	return inst;
}

