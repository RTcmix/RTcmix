#include <stdlib.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "START.h"
#include <rt.h>
#include <rtdefs.h>

strumq *curstrumq[6];

extern "C" {
	void sset(float, float, float, strumq*);
	void randfill(float, int, strumq*);
	float strum(float, strumq*);
}

START::START() : Instrument()
{
	// future setup here?
}

START::~START()
{
	if (deleteflag == 1) {
		delete strumq1;
	}
}

// p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
// p4 = nyquist decay time; p5 = amp, p6 = squish; p7 = stereo spread [optional]
// p8 = flag for deleting pluck arrays (used by FRET, BEND, etc.) [optional]

int START::init(double p[], int n_args)
{
	int	squish;
	float outskip, dur, pitch, fdecay, nydecay, amp, freq;

	outskip = p[0];
	dur = p[1];
	pitch = p[2];
	fdecay = p[3];
	nydecay = p[4];
	amp = p[5];
	squish = (int)p[6];
	spread = p[7];
	deleteflag = (int)p[8];

	rtsetoutput(outskip, dur, this);

	strumq1 = new strumq;
	curstrumq[0] = strumq1;
	freq = cpspch(pitch);
	sset(freq, fdecay, nydecay, strumq1);
	randfill(amp, squish, strumq1);

   amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, dur, amplen, amptabs);
	}
	else
		advise("START", "Setting phrase curve to all 1's.");

	skip = (int)(SR / (float)resetval);

	return(nsamps);
}

int START::run()
{
	int i, branch;
	float aamp, out[2];

	aamp = 1.0;                  /* in case amptable == NULL */

	branch = 0;
	for (i = 0; i < chunksamps; i++) {
		if (--branch < 0) {
			if (amptable)
				aamp = tablei(cursamp, amptable, amptabs);
			branch = skip;
		}
		out[0] = strum(0.,strumq1) * aamp;

		if (outputchans == 2) { /* split stereo files between the channels */
			out[1] = (1.0 - spread) * out[0];
			out[0] *= spread;
			}

		rtaddout(out);
		cursamp++;
	}
	return i;
}

Instrument*
makeSTART()
{
	START *inst;

	inst = new START();
	inst->set_bus_config("START");

	return inst;
}

