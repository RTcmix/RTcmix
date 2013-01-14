#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "LSFLUTE.h"
#include <rt.h>
#include <rtdefs.h>
#include "sflcfuncs.h"


// these are from "LSFLUTE"
extern int *dl1ptr,*dl2ptr;
extern float *del1ptr,*del2ptr;
extern int olength1,olength2;

extern "C" {
// BGG -- I don't think this function exists?
	void mdelpartset(float*, int*, int);
}

LSFLUTE::LSFLUTE() : Instrument()
{
	branch = 0;
}

int LSFLUTE::init(double p[], int n_args)
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
		return die("LSFLUTE", "You haven't made the noise amp envelope (table 1).");

	oamparr = floc(2);
	if (oamparr) {
		int len = fsize(2);
		tableset(SR, p[1], len, oamptabs);
	}
	else
		return die("LSFLUTE", "You haven't made the output amp envelope (table 2).");

//	int imax = DELSIZE;
//	mdelpartset(del1ptr,dl1ptr,imax);
//	mdelpartset(del2ptr,dl2ptr,imax);

//	srrand(0.1);
	length1 = (int)p[3];
	length2 = (int)p[4];

	oldsig = 0; /* for the filter */

	amp = p[5];
	namp = p[2];
	spread = p[6];
	skip = (int)(SR/(float)resetval);

	aamp = oamp = 0.0;

	return nSamps();
}

int LSFLUTE::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (olength1 < length1) olength1++;
		if (olength1 > length1) olength1--;
		if (olength2 < length2) olength2++;
		if (olength2 > length2) olength2--;
		if (--branch <= 0) {
			aamp = tablei(currentFrame(), amparr, amptabs);
			oamp = tablei(currentFrame(), oamparr, oamptabs);
			branch = skip;
		}

		float sig = (rrand() * namp * aamp) + aamp;
		float del1sig = mdelget(del1ptr,olength1,dl1ptr);
		sig = sig + (del1sig * -0.35);
#ifdef MAXMSP
		delput(sig,del2ptr,dl2ptr);
#else
		mdelput(sig,del2ptr,dl2ptr);
#endif

		sig = mdelget(del2ptr,olength2,dl2ptr);
		sig = (sig * sig * sig) - sig;
		sig = (0.4 * sig) + (0.9 * del1sig);

		float out[2];
		out[0] = sig * amp * oamp;
		sig = (dampcoef * sig) + ((1.0 - dampcoef) * oldsig);
		oldsig = sig;
#ifdef MAXMSP
		delput(sig,del1ptr,dl1ptr);
#else
		mdelput(sig,del1ptr,dl1ptr);
#endif

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
makeLSFLUTE()
{
	LSFLUTE *inst;

	inst = new LSFLUTE();
	inst->set_bus_config("LSFLUTE");

	return inst;
}
