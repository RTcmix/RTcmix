#include <iostream.h>
#include <mixerr.h>
#include <Instrument.h>
#include "START1.h"
#include <rt.h>
#include <rtdefs.h>

extern strumq *curstrumq[6];
delayq *curdelayq;

extern "C" {
	#include <ugens.h>
	void sset(float, float, float, strumq*);
	void randfill(float, int, strumq*);
	float strum(float, strumq*);
	void delayset(float, delayq*);
	void delayclean(delayq*);
	float dist(float);
	float delay(float, delayq*);
}

START1::START1() : Instrument()
{
	// future setup here?
}

START1::~START1()
{
	if (deleteflag == 1) {
		delete strumq1;
		delete dq;
	}
}

int START1::init(float p[], short n_args)
{
// p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
// p4 = nyquist decay time; p5 = distortion gain; p6 = feedback gain
// p7 = feedback pitch (oct.pc); p8 = clean signal level
// p9 = distortion signal level; p10 = amp; p11 = squish
// p12 = stereo spread [optional]
// p13 = flag for deleting pluck arrays (used by FRET, BEND, etc.) [optional]

	float freq;

	nsamps = rtsetoutput(p[0], p[1], this);
	 
	strumq1 = new strumq;
	curstrumq[0] = strumq1;
	freq = cpspch(p[2]);
	sset(freq, p[3], p[4], strumq1);
	randfill(1.0, (int)p[11], strumq1);

	dq = new delayq;
	curdelayq = dq;
	delayset(cpspch(p[7]), dq);
	delayclean(dq);

	dgain = p[5];
	fbgain = p[6]/dgain;
	cleanlevel = p[8];
	distlevel = p[9];
	amp = p[10];
	spread = p[12];
	deleteflag = p[13];

	d = 0.0;

	return(nsamps);
}

int START1::run()
{
	int i;
	float out[2];
	float a,b;

	for (i = 0; i < chunksamps; i++) {
		a = strum(d, strumq1);
		b = dist(dgain*a);
		d = fbgain*delay(b, dq);

		out[0] = (cleanlevel*a + distlevel*b) * amp;

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
makeSTART1()
{
	START1 *inst;

	inst = new START1();
	return inst;
}
