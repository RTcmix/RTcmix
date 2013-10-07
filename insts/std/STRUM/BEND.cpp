#include <ugens.h>
#include <Instrument.h>
#include "BEND.h"
#include <rt.h>
#include <rtdefs.h>

extern StrumQueue *curstrumq[6];

extern "C" {
	void sset(float, float, float, float, strumq*);
	float strum(float, strumq*);
}

BEND::BEND() : Instrument()
{
	branch = 0;
}

BEND::~BEND()
{
// BGGx -- deleting the strumq disables the 'carry-over' necessary for
// BEND/FRET/etc. to work.  There has to be a better way, but this is
// an ancient instrument
//	strumq1->unref();
}

int BEND::init(double p[], int n_args)
{
// p0 = start; p1 = dur; p2 = pitch0 (oct.pc); p3 = pitch1 (oct.pc);
// p4 = gliss function; p5 = fundamental decay time; p6 = nyquist decay time;
// p7 = update every nsamples; p8 = stereo spread [optional]

	float dur = p[1];
	if (rtsetoutput(p[0], dur, this) == -1)
		return DONT_SCHEDULE;

	strumq1 = curstrumq[0];
	strumq1->ref();
	freq0 = cpspch(p[2]);
	freq1 = cpspch(p[3]);
	diff = freq1 - freq0;

	tf0 = p[5];
	tfN = p[6];
	sset(SR, freq0, tf0, tfN, strumq1);

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, dur, amplen, amptabs);
	}
	else {
		rtcmix_advise("BEND", "Setting phrase curve to all 1's.");
		aamp = 1.0;
	}

	glissf = floc((int)p[4]);
	if (glissf) {
		int leng = fsize((int)p[4]);
		tableset(SR, p[1],leng,tags);
	}
	else
		return die("BEND", "You haven't made the glissando function (table %d).",
						(int)p[4]);

	reset = (int)p[7];
	if (reset == 0) reset = 100;
	spread = p[8];

	return nSamps();
}

int BEND::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			if (amptable)
				aamp = tablei(currentFrame(), amptable, amptabs);
			float freq = diff * tablei(currentFrame(), glissf, tags) + freq0;
			sset(SR, freq, tf0, tfN, strumq1);
			branch = reset;
		}

		float out[2];
		out[0] = strum(0.,strumq1) * aamp;

		if (outputChannels() == 2) {
			out[1] = (1.0 - spread) * out[0];
			out[0] *= spread;
		}

		rtaddout(out);
		increment();
	}
	return framesToRun();
}



Instrument*
makeBEND()
{
	BEND *inst;

	inst = new BEND();
	inst->set_bus_config("BEND");

	return inst;
}
