#include <ugens.h>
#include <Instrument.h>
#include "FRET.h"
#include <rt.h>
#include <rtdefs.h>

extern StrumQueue *curstrumq[6];

extern "C" {
	void sset(float, float, float, float, strumq*);
	float strum(float, strumq*);
}

FRET::FRET() : Instrument()
{
	branch = 0;
}

FRET::~FRET()
{
// BGGx -- deleting the strumq disables the 'carry-over' necessary for
// BEND/FRET/etc. to work.  There has to be a better way, but this is
// an ancient instrument
//	strumq1->unref();
}

int FRET::init(double p[], int n_args)
{
// p0 = start; p1 = dur; p2 = pitch(oct.pc); p3 = fundamental decay time;
// p4 = nyquist decay time; p5 = stereo spread [optional]

	float	dur = p[1];

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	strumq1 = curstrumq[0];
	strumq1->ref();
	freq = cpspch(p[2]);
	tf0 = p[3];
	tfN = p[4];

	spread = p[5];

   amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, dur, amplen, amptabs);
	}
	else {
		rtcmix_advise("FRET", "Setting phrase curve to all 1's.");
		aamp = 1.0;
	}

	skip = (int)(SR / (float)resetval);

	return nSamps();
}

int FRET::configure()
{
	sset(SR, freq, tf0, tfN, strumq1);
	return 0;
}

int FRET::run()
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
makeFRET()
{
	FRET *inst;

	inst = new FRET();
	inst->set_bus_config("FRET");

	return inst;
}
