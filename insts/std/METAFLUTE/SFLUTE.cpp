#include <iostream.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "SFLUTE.h"
#include <rt.h>
#include <rtdefs.h>


// these are for "LSFLUTE"
int *dl1ptr,*dl2ptr;
float *del1ptr,*del2ptr;
int olength1,olength2;

extern "C" {
	void mdelset(float*, int*, int);
	float mdelget(float*, int, int*);
}

SFLUTE::SFLUTE() : Instrument()
{
	// future setup here?
}

int SFLUTE::init(float p[], int n_args)
{
// p0 = start; p1 = dur; p2 = noise amp; p3 = length1; p4 = length2
// p5 = amp multiplier; p6 = stereo spread (0-1) <optional>
// function slot 1 is the noise amp envelope
// function slot 2 is the out amp envelope

	int imax;

	nsamps = rtsetoutput(p[0], p[1], this);

	dampcoef = .7;

	amparr = floc(1);
	lenamp = fsize(1);
	tableset(p[1], lenamp, amptabs);

	oamparr = floc(2);
	olenamp = fsize(2);
	tableset(p[1], olenamp, oamptabs);

	imax = DELSIZE;
	mdelset(del1,dl1,imax);
	mdelset(del2,dl2,imax);
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

	return(nsamps);
}

int SFLUTE::run()
{
	int i;
	float out[2];
	float aamp,oamp;
	float sig,del1sig;
	int branch;

	Instrument::run();

	branch = 0;
	for (i = 0; i < chunksamps; i++) {
		if (--branch < 0) {
			aamp = tablei(cursamp, amparr, amptabs);
			oamp = tablei(cursamp, oamparr, oamptabs);
			branch = skip;
			}

		sig = (rrand() * namp * aamp) + aamp;
		del1sig = mdelget(del1,length1,dl1);
		sig = sig + (del1sig * -0.35);
		delput(sig,del2,dl2);

		sig = mdelget(del2,length2,dl2);
		sig = (sig * sig * sig) - sig;
		sig = (0.4 * sig) + (0.9 * del1sig);
		out[0] = sig * amp * oamp;
		sig = (dampcoef * sig) + ((1.0 - dampcoef) * oldsig);
		oldsig = sig;
		delput(sig,del1,dl1);

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
makeSFLUTE()
{
	SFLUTE *inst;

	inst = new SFLUTE();
	inst->set_bus_config("SFLUTE");

	return inst;
}
