#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "VFRET1.h"
#include <rt.h>
#include <rtdefs.h>

extern strumq *curstrumq[6];
extern delayq *curdelayq;

extern "C" {
	void sset(float, float, float, strumq*);
	float strum(float, strumq*);
	void delayset(float, delayq*);
	float dist(float);
	float delay(float, delayq*);
}

VFRET1::VFRET1() : Instrument()
{
	branch1 = branch2 = 0;
}

int VFRET1::init(float p[], int n_args)
{
// p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
// p4 = nyquist decay time; p5 = distortion gain; p6 = feedback gain
// p7 = feedback pitch (oct.pc); p8 = clean signal level
// p9 = distortion signal level; p10 = amp; p11 = low vibrato freq range;
// p12 = hi vibrato freq range; p13 = vibrato freq depth (expressed in cps);
// p14 = pitch update (default 200/sec); p15 = stereo spread [optional]
// assumes makegen 1 is the amplitude envelope, makegen 2 is the vibrato
// function, and makegen 3 is the vibrato amplitude envelope

	int elen;

	nsamps = rtsetoutput(p[0], p[1], this);

	strumq1 = curstrumq[0];
	freq = cpspch(p[2]);
	tf0 = p[3];
	tfN = p[4];

	dq = curdelayq;
	fbpitch = cpspch(p[7]);

	amp = p[10];
	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(p[1], amplen, amptabs);
	}
	else {
		advise("VFRET1", "Setting phrase curve to all 1's.");
		aamp = amp;
	}

	vloc = floc(2);
	if (vloc == NULL)
		die("VFRET1", "You need to store a vibrato function in gen num. 2.");
	vlen = fsize(2);

	vsibot = p[11] * (float)vlen/SR;
	vsidiff = vsibot - (p[12] * (float)vlen/SR);
	vsi = ((rrand()+1.0)/2.0) * vsidiff;
	vsi += vsibot;
	vphase = 0.0;

	eloc = floc(3);
	if (eloc == NULL)
		die("VFRET1", "You need to store a vibrato amp. envelope in gen num. 3.");
	elen = fsize(3);
	tableset(p[1], elen, tab);

	dgain = p[5];
	fbgain = p[6]/dgain;
	cleanlevel = p[8];
	distlevel = p[9];
	vdepth = p[13];
	reset= (int)p[14];
	if (reset == 0) reset = 200;
	spread = p[15];

	firsttime = 1;
	d = 0.0;

	return(nsamps);
}

int VFRET1::run()
{
	int i;
	float out[2];
	float a,b;
	float vamp;
	float freqch;

	Instrument::run();

	if (firsttime) {
		sset(freq, tf0, tfN, strumq1);
		delayset(fbpitch, dq);
		firsttime = 0;
		}

	for (i = 0; i < chunksamps; i++) {
		if (--branch1 < 0) {
			vsi = (( (rrand()+1.0)/2.0) * vsidiff) + vsibot;
			branch1 = (int)((float)vlen/vsi);
			}
		if (--branch2 < 0) {
			if (amptable)
				aamp = tablei(cursamp, amptable, amptabs) * amp;
			vamp = tablei(cursamp, eloc, tab) * vdepth;
			freqch = oscili(vamp,vsi,vloc,vlen,&vphase);
			sset(freq+freqch, tf0, tfN, strumq1);
			branch2 = reset;
			vphase += (float)branch2 * vsi;
			while (vphase >= (float) vlen)
				vphase -= (float) vlen;
			}

		a = strum(d, strumq1);
		b = dist(dgain*a);
		d = fbgain*delay(b, dq);

		out[0] = (cleanlevel*a + distlevel*b) * aamp;

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
makeVFRET1()
{
	VFRET1 *inst;

	inst = new VFRET1();
	inst->set_bus_config("VFRET1");

	return inst;
}
