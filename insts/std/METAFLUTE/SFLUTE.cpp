#include <ugens.h>
#include <Instrument.h>
#include "SFLUTE.h"
#include <rt.h>
#include <rtdefs.h>
#include "sflcfuncs.h"

// these are for "LSFLUTE"
int *dl1ptr,*dl2ptr;
float *del1ptr,*del2ptr;
int olength1,olength2;

SFLUTE::SFLUTE() : Instrument()
{
	branch = 0;
}

int SFLUTE::init(double p[], int n_args)
{
// p0 = start; p1 = dur; p2 = noise amp; p3 = length1; p4 = length2
// p5 = amp multiplier; p6 = stereo spread (0-1) <optional>
// function slot 1 is the noise amp envelope
// function slot 2 is the out amp envelope

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	dampcoef = .7;

	amparr = floc(1);
	if (amparr) {
		int len = fsize(1);
		tableset(SR, p[1], len, amptabs);
	}
	else
		return die("SFLUTE", "You haven't made the noise amp envelope (table 1).");

	oamparr = floc(2);
	if (oamparr) {
		int len = fsize(2);
		tableset(SR, p[1], len, oamptabs);
	}
	else
		return die("SFLUTE", "You haven't made the output amp envelope (table 2).");

	int imax = DELSIZE;
	mdelset(SR, del1,dl1,imax);
	mdelset(SR, del2,dl2,imax);
	dl1ptr = dl1;
	dl2ptr = dl2;
	del1ptr = del1;
	del2ptr = del2;

//	srrand(0.1);
	length1 = (int)p[3];
	length2 = (int)p[4];
	olength1 = length1;
	olength2 = length2;

	oldsig = 0; /* for the filter */

	amp = p[5];
	namp = p[2];
	spread = p[6];
	skip = (int)(SR/(float)resetval);

	aamp = oamp = 0.0;

	return nSamps();
}

int SFLUTE::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			aamp = tablei(currentFrame(), amparr, amptabs);
			oamp = tablei(currentFrame(), oamparr, oamptabs);
			branch = skip;
		}

		float sig = (rrand() * namp * aamp) + aamp;
		float del1sig = mdelget(del1,length1,dl1);
		sig = sig + (del1sig * -0.35);
// BGG mm -- delput works fine
//		mdelput(sig,del2,dl2);
		delput(sig,del2,dl2);

		sig = mdelget(del2,length2,dl2);
		sig = (sig * sig * sig) - sig;
		sig = (0.4 * sig) + (0.9 * del1sig);

		float out[2];
		out[0] = sig * amp * oamp;
		sig = (dampcoef * sig) + ((1.0 - dampcoef) * oldsig);
		oldsig = sig;
// BGG mm -- delput works fine
//		mdelput(sig,del1,dl1);
		delput(sig,del1,dl1);

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
makeSFLUTE()
{
	SFLUTE *inst;

	inst = new SFLUTE();
	inst->set_bus_config("SFLUTE");

	return inst;
}
