#include <iostream.h>
#include <mixerr.h>
#include <Instrument.h>
#include "START.h"
#include <rt.h>
#include <rtdefs.h>

strumq *curstrumq[6];

extern "C" {
	#include <ugens.h>
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

int START::init(float p[], short n_args)
{
// p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
// p4 = nyquist decay time; p5 = amp, p6 = squish; p7 = stereo spread [optional]
// p8 = flag for deleting pluck arrays (used by FRET, BEND, etc.) [optional]

	float freq;

	rtsetoutput(p[0], p[1], this);

	strumq1 = new strumq;
	curstrumq[0] = strumq1;
	freq=cpspch(p[2]);
	sset(freq, p[3], p[4], strumq1);
	randfill(p[5], (int)p[6], strumq1);

	spread = p[7];
	deleteflag = p[8];

	return(nsamps);
}

int START::run()
{
	int i;
	float out[2];

	for (i = 0; i < chunksamps; i++) {
		out[0] = strum(0.,strumq1);

		if (NCHANS == 2) { /* split stereo files between the channels */
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
	return inst;
}

