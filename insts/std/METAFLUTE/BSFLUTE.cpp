#include <ugens.h>
#include <Instrument.h>
#include "BSFLUTE.h"
#include <rt.h>
#include <rtdefs.h>

extern "C" {
	void mdelset(float, float*, int*, int);
	float mdliget(float*, float, int*);
}

BSFLUTE::BSFLUTE() : Instrument()
{
	branch = 0;
}

int BSFLUTE::init(double p[], int n_args)
{
// p0 = start; p1 = dur; p2 = noise amp; p3 = length1low; p4 = length1high
// p5 = length2low; p6 = length2high; p7 = amp multiplier; 
// p8 = stereo spread (0-1) <optional>
// function slot 1 is the noise amp envelope
// function slot 2 is the out amp envelope
// function slot 3 is the pitch-tracking curve for length 1
// function slot 4 is the pitch-tracking curve for length 2

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	dampcoef = .7;

	amparr = floc(1);
	if (amparr) {
		int len = fsize(1);
		tableset(SR, p[1], len, amptabs);
	}
	else
		return die("BSFLUTE", "You haven't made the noise amp envelope (table 1).");

	oamparr = floc(2);
	if (oamparr) {
		int len = fsize(2);
		tableset(SR, p[1], len, oamptabs);
	}
	else
		return die("BSFLUTE", "You haven't made the output amp envelope (table 2).");

	pcurve1 = floc(3);
	if (pcurve1) {
		int len = fsize(3);
		tableset(SR, p[1], len, ptabs1);
	}
	else
		return die("BSFLUTE", "You haven't made the pitch-tracking curve for "
					"length 1 (table 3).");

	pcurve2 = floc(4);
	if (pcurve2) {
		int len = fsize(4);
		tableset(SR, p[1], len, ptabs2);
	}
	else
		return die("BSFLUTE", "You haven't made the pitch-tracking curve for "
					"length 2 (table 4).");

	int imax = DELSIZE;
	mdelset(SR, del1,dl1,imax);
	mdelset(SR, del2,dl2,imax);

//	srrand(0.1);

	l1base = p[3];
	l2base = p[5];
	l1span = p[4] - p[3];
	l2span = p[6] - p[5];

	oldsig = 0; /* for the filter */

	amp = p[7];
	namp = p[2];
	spread = p[8];
	skip = (int)(SR/(float)resetval);

	aamp = oamp = 0.0;
	length1 = length2 = 0.0;

	return nSamps();
}

int BSFLUTE::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			aamp = tablei(currentFrame(), amparr, amptabs);
			oamp = tablei(currentFrame(), oamparr, oamptabs);
			length1 = l1base + (l1span * tablei(currentFrame(),pcurve1,ptabs1));
			length2 = l2base + (l2span * tablei(currentFrame(),pcurve2,ptabs2));
			branch = skip;
		}

		float sig = (rrand() * namp * aamp) + aamp;
		float del1sig = mdliget(del1,length1,dl1);
		sig = sig + (del1sig * -0.35);
		delput(sig,del2,dl2);

		sig = mdliget(del2,length2,dl2);
		sig = (sig * sig * sig) - sig;
		sig = (0.4 * sig) + (0.9 * del1sig);

		float out[2];
		out[0] = sig * amp * oamp;
		sig = (dampcoef * sig) + ((1.0 - dampcoef) * oldsig);
		oldsig = sig;
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
makeBSFLUTE()
{
	BSFLUTE *inst;

	inst = new BSFLUTE();
	inst->set_bus_config("BSFLUTE");

	return inst;
}
