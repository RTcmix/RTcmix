#include <stdlib.h>
#include <stdio.h>
#include <ugens.h>
#include <Instrument.h>
#include "VFRET1.h"
#include <rt.h>
#include <rtdefs.h>

extern StrumQueue *curstrumq[6];
extern DelayQueue *curdelayq;

extern "C" {
	void sset(float, float, float, float, strumq*);
	float strum(float, strumq*);
	void delayset(float, float, delayq*);
	float dist(float);
	float delay(float, delayq*);
}

VFRET1::VFRET1() : Instrument()
{
	branch1 = branch2 = 0;
}

VFRET1::~VFRET1()
{
// BGGx -- deleting the strumq (and dq) disables the 'carry-over' necessary
// for BEND/FRET/etc. to work.  There has to be a better way, but this is
// an ancient instrument
//	strumq1->unref();
//	dq->unref();
}

int VFRET1::init(double p[], int n_args)
{
// p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
// p4 = nyquist decay time; p5 = distortion gain; p6 = feedback gain
// p7 = feedback pitch (oct.pc); p8 = clean signal level
// p9 = distortion signal level; p10 = amp; p11 = low vibrato freq range;
// p12 = hi vibrato freq range; p13 = vibrato freq depth (expressed in cps);
// p14 = pitch update (default 200/sec); p15 = stereo spread [optional]
// assumes makegen 1 is the amplitude envelope, makegen 2 is the vibrato
// function, and makegen 3 is the vibrato amplitude envelope

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	strumq1 = curstrumq[0];
	strumq1->ref();
	
	freq = cpspch(p[2]);
	tf0 = p[3];
	tfN = p[4];

	dq = curdelayq;
	dq->ref();
	fbpitch = cpspch(p[7]);

	amp = p[10];
	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, p[1], amplen, amptabs);
	}
	else {
		rtcmix_advise("VFRET1", "Setting phrase curve to all 1's.");
		aamp = amp;
	}

	vloc = floc(2);
	if (vloc == NULL)
		return die("VFRET1", "You need to store a vibrato function in gen num. 2.");
	vlen = fsize(2);

	vsibot = p[11] * (float)vlen/SR;
	vsidiff = vsibot - (p[12] * (float)vlen/SR);
	vsi = ((rrand()+1.0)/2.0) * vsidiff;
	vsi += vsibot;
	vphase = 0.0;

	eloc = floc(3);
	if (eloc == NULL)
		return die("VFRET1", "You need to store a vibrato amp. envelope in gen num. 3.");
	int elen = fsize(3);
	tableset(SR, p[1], elen, tab);

	dgain = p[5];
	fbgain = p[6]/dgain;
	cleanlevel = p[8];
	distlevel = p[9];
	vdepth = p[13];
	reset= (int)p[14];
	if (reset == 0) reset = 200;
	spread = p[15];

	d = 0.0;

	return nSamps();
}

int VFRET1::configure()
{
	sset(SR, freq, tf0, tfN, strumq1);
	delayset(SR, fbpitch, dq);
	return 0;
}

int VFRET1::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--branch1 <= 0) {
			vsi = (( (rrand()+1.0)/2.0) * vsidiff) + vsibot;
			branch1 = (int)((float)vlen/vsi);
		}
		if (--branch2 <= 0) {
			if (amptable)
				aamp = tablei(currentFrame(), amptable, amptabs) * amp;
			float vamp = tablei(currentFrame(), eloc, tab) * vdepth;
			float freqch = oscili(vamp,vsi,vloc,vlen,&vphase);
			sset(SR, freq+freqch, tf0, tfN, strumq1);
			branch2 = reset;
			vphase += (float)branch2 * vsi;
			while (vphase >= (float) vlen)
				vphase -= (float) vlen;
		}

		float a = strum(d, strumq1);
		float b = dist(dgain*a);
		d = fbgain*delay(b, dq);

		float out[2];
		out[0] = (cleanlevel*a + distlevel*b) * aamp;

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
makeVFRET1()
{
	VFRET1 *inst;

	inst = new VFRET1();
	inst->set_bus_config("VFRET1");

	return inst;
}
