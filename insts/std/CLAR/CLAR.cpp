#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "CLAR.h"
#include <rt.h>
#include <rtdefs.h>

extern "C" {
	/* defined in cfuncs.c */
	void mdelset(float*, int*, int);
	float mdelget(float*, int, int*);
}

CLAR::CLAR() : Instrument()
{
	// future setup here?
}

int CLAR::init(float p[], int n_args)
{
// p0 = start; p1 = dur; p2 = noise amp; p3 = length1; p4 = length2
// p5 = output amp; p6 = d2 gain; p7 = stereo spread (0-1) <optional>
// function slot 1 is the noise amp envelope
// function slot 2 is the output amp envelope

	int imax;

	nsamps = rtsetoutput(p[0], p[1], this);

	dampcoef = .7;

	amparr = floc(1);
	if (amparr) {
		int lenamp = fsize(1);
		tableset(p[1], lenamp, amptabs);
	}
	else
		advise("CLAR", "Setting noise amp curve to all 1's.");

	oamparr = floc(2);
	if (oamparr) {
		int olenamp = fsize(2);
		tableset(p[1], olenamp, oamptabs);
	}
	else
		advise("CLAR", "Setting output amp curve to all 1's.");

	imax = DELSIZE;
	mdelset(del1,dl1,imax);
	mdelset(del2,dl2,imax);

//	srrand(0.1);
	length1 = (int)p[3];
	length2 = (int)p[4];

	oldsig = 0; /* for the filter */

	amp = p[5];
	namp = p[2];
	d2gain = p[6];
	spread = p[7];
	skip = (int)(SR/(float)resetval);

	return(nsamps);
}

int CLAR::run()
{
	int i;
	float out[2];
	float aamp,oamp;
	float sig,del1sig;
	float del2sig,csig,ssig;
	int branch;

	Instrument::run();

	aamp = 1.0;        /* in case amparr == NULL */

	branch = 0;
	for (i = 0; i < chunksamps; i++) {
		if (--branch < 0) {
			if (amparr)
				aamp = table(cursamp, amparr, amptabs);
			if (oamparr)
				oamp = tablei(cursamp, oamparr, oamptabs);
			else
				oamp = 1.0;
			branch = skip;
			}

		sig = (rrand() * namp * aamp) + aamp;
		del1sig = mdelget(del1,length1,dl1);
		del2sig = mdelget(del2,length2,dl2);
		if (del1sig > 1.0) del1sig = 1.0;
		if (del1sig < -1.0) del1sig = -1.0;
		if (del2sig > 1.0) del2sig = 1.0;
		if (del2sig < -1.0) del2sig = -1.0;
		sig = sig + 0.9 * ((d2gain * del2sig) + ((0.9-d2gain) * del1sig));
		csig = -0.5 * sig + aamp;
		ssig = sig * sig;
		sig = (0.3 * ssig) + (-0.8 * (sig * ssig));
		sig = sig + csig;
		sig = (0.7 * sig) + (0.3 * oldsig);
		oldsig = sig;
		delput(sig,del2,dl2);
		delput(sig,del1,dl1);
		out[0] = sig * amp * oamp;
		if (outputchans == 2) {
			out[1] = (1.0 - spread) * out[0];
			out[0] *= spread;
			}

		rtaddout(out);
		cursamp++;
		}
	return i;
}



Instrument*
makeCLAR()
{
	CLAR *inst;

	inst = new CLAR();
	inst->set_bus_config("CLAR");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("CLAR",makeCLAR);
}

