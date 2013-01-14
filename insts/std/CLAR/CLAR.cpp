#include <stdio.h>
#include <ugens.h>
#include <Instrument.h>
#include "CLAR.h"
#include <rt.h>
#include <rtdefs.h>

extern "C" {
	/* defined in cfuncs.c */
	void mdelset(float, float*, int*, int);
	float mdelget(float*, int, int*);
}

CLAR::CLAR() : Instrument()
{
	branch = 0;
}

int CLAR::init(double p[], int n_args)
{
// p0 = start; p1 = dur; p2 = noise amp; p3 = length1; p4 = length2
// p5 = output amp; p6 = d2 gain; p7 = stereo spread (0-1) <optional>
// function slot 1 is the noise amp envelope
// function slot 2 is the output amp envelope

	int imax;

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	dampcoef = .7;

	amparr = floc(1);
	if (amparr) {
		int lenamp = fsize(1);
		tableset(SR, p[1], lenamp, amptabs);
	}
	else
		rtcmix_advise("CLAR", "Setting noise amp curve to all 1's.");

	oamparr = floc(2);
	if (oamparr) {
		int olenamp = fsize(2);
		tableset(SR, p[1], olenamp, oamptabs);
	}
	else
		rtcmix_advise("CLAR", "Setting output amp curve to all 1's.");

	imax = DELSIZE;
	mdelset(SR, del1,dl1,imax);
	mdelset(SR, del2,dl2,imax);

//	srrand(0.1);
	length1 = (int)p[3];
	length2 = (int)p[4];

	oldsig = 0; /* for the filter */

	amp = p[5];
	namp = p[2];
	d2gain = p[6];
	spread = p[7];
	skip = (int)(SR/(float)resetval);

	aamp = oamp = 1.0;        /* in case amparr or oamparr are NULL */

	return nSamps();
}

int CLAR::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			if (amparr) {
#ifdef MAXMSP
				aamp = rtcmix_table(currentFrame(), amparr, amptabs);
#else
				aamp = table(currentFrame(), amparr, amptabs);
#endif
			}
			if (oamparr)
				oamp = tablei(currentFrame(), oamparr, oamptabs);
			branch = skip;
		}

		float sig = (rrand() * namp * aamp) + aamp;
		float del1sig = mdelget(del1,length1,dl1);
		float del2sig = mdelget(del2,length2,dl2);
		if (del1sig > 1.0) del1sig = 1.0;
		if (del1sig < -1.0) del1sig = -1.0;
		if (del2sig > 1.0) del2sig = 1.0;
		if (del2sig < -1.0) del2sig = -1.0;
		sig = sig + 0.9 * ((d2gain * del2sig) + ((0.9-d2gain) * del1sig));
		float csig = -0.5 * sig + aamp;
		float ssig = sig * sig;
		sig = (0.3 * ssig) + (-0.8 * (sig * ssig));
		sig = sig + csig;
		sig = (0.7 * sig) + (0.3 * oldsig);
		oldsig = sig;
		delput(sig,del2,dl2);
		delput(sig,del1,dl1);

		float out[2];
		out[0] = sig * amp * oamp;
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
makeCLAR()
{
	CLAR *inst;

	inst = new CLAR();
	inst->set_bus_config("CLAR");

	return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("CLAR",makeCLAR);
}
#endif
