#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "VSTART1.h"
#include <rt.h>
#include <rtdefs.h>

extern strumq *curstrumq[6];
extern delayq *curdelayq;

extern "C" {
	void sset(float, float, float, strumq*);
	void randfill(float, int, strumq*);
	float strum(float, strumq*);
	void delayset(float, delayq*);
	void delayclean(delayq*);
	float dist(float);
	float delay(float, delayq*);
}

VSTART1::VSTART1() : Instrument()
{
	// future setup here?
}

VSTART1::~VSTART1()
{
	if (deleteflag == 1) {
		delete strumq1;
		delete dq;
	}
}

int VSTART1::init(float p[], short n_args)
{
// p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
// p4 = nyquist decay time; p5 = distortion gain; p6 = feedback gain
// p7 = feedback pitch (oct.pc); p8 = clean signal level
// p9 = distortion signal level; p10 = amp; p11 = squish
// p12 = low vibrato freq range; p13 = hi vibrato freq range
// p14 = vibrato freq depth (expressed in cps); p15 = random seed value
// p16 = pitch update (default 200/sec)
// p17 = stereo spread [optional]
// p18 = flag for deleting pluck arrays (used by FRET, BEND, etc.) [optional]
// assumes makegen 1 is the vibrato function and makegen 2 is the
// vibrato amplitude envelope

	int elen;

	nsamps = rtsetoutput(p[0], p[1], this);

	strumq1 = new strumq;
	curstrumq[0] = strumq1;
	freq = cpspch(p[2]);
	tf0 = p[3];
	tfN = p[4];
	sset(freq, tf0, tfN, strumq1);
	randfill(1.0, (int)p[11], strumq1);

	dq = new delayq;
	curdelayq = dq;
	delayset(cpspch(p[7]), dq);
	delayclean(dq);

	vloc = floc(1);
	if (vloc == NULL)
		die("VSTART1", "You need to store a vibrato function in gen num. 1.");

	vlen = fsize(1);
	vsibot = p[12] * (float)vlen/SR;
	vsidiff = vsibot - (p[13] * (float)vlen/SR);
	srrand((int)p[15]);
	vsi = ((rrand()+1.0)/2.0) * vsidiff;
	vsi += vsibot;
	vphase = 0.0;

	elen = fsize(2);
	eloc = floc(2);
	tableset(p[1], elen, tab);

	dgain = p[5];
	fbgain = p[6]/dgain;
	cleanlevel = p[8];
	distlevel = p[9];
	amp = p[10];
	vdepth = p[14];
	reset = (int)p[16];
	if (reset == 0) reset = 200;
	spread = p[17];
	deleteflag = (int)p[18];

	d = 0.0;

	return(nsamps);
}

int VSTART1::run()
{
	int i;
	float out[2];
	float a,b;
	float vamp;
	float freqch;
	int branch1,branch2;

	Instrument::run();

	branch1 = branch2 = 0;
	for (i = 0; i < chunksamps; i++) {
		if (--branch1 < 0) {
			vsi = (( (rrand()+1.0)/2.0) * vsidiff) + vsibot;
			branch1 = (int)((float)vlen/vsi);
			}
		if (--branch2 < 0) {
			vamp = tablei(cursamp, eloc, tab) * vdepth;
			freqch = oscili(vamp,vsi,vloc,vlen,&vphase);
			sset(freq+freqch, tf0, tfN, strumq1);
			branch2 = reset;
			vphase += (float)branch2 * vsi;
			}

		a = strum(d, strumq1);
		b = dist(dgain*a);
		d = fbgain*delay(b, dq);

		out[0] = (cleanlevel*a + distlevel*b) * amp;

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
makeVSTART1()
{
	VSTART1 *inst;

	inst = new VSTART1();
	inst->set_bus_config("VSTART1");

	return inst;
}
