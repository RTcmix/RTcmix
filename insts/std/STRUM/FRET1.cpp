#include <iostream.h>
#include "../../sys/mixerr.h"
#include "../../rtstuff/Instrument.h"
#include "FRET1.h"
#include "../../rtstuff/rt.h"
#include "../../rtstuff/rtdefs.h"

extern strumq *curstrumq[6];
extern delayq *curdelayq;

extern "C" {
	#include "../../H/ugens.h"
	void sset(float, float, float, strumq*);
	float strum(float, strumq*);
	void delayset(float, delayq*);
	float dist(float);
	float delay(float, delayq*);
}

FRET1::FRET1() : Instrument()
{
	// future setup here?
}

int FRET1::init(float p[], short n_args)
{
// p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
// p4 = nyquist decay time; p5 = distortion gain; p6 = feedback gain
// p7 = feedback pitch (oct.pc); p8 = clean signal level
// p9 = distortion signal level; p10 = amp; p11 = stereo spread [optional]

	nsamps = rtsetoutput(p[0], p[1], this);

	strumq1 = curstrumq[0];
	freq = cpspch(p[2]);
	tf0 = p[3];
	tfN = p[4];

	dq = curdelayq;
	fbpitch = cpspch(p[7]);

	dgain = p[5];
	fbgain = p[6]/dgain;
	cleanlevel = p[8];
	distlevel = p[9];
	amp = p[10];
	spread = p[11];

	firsttime = 1;
	d = 0.0;

	return(nsamps);
}

int FRET1::run()
{
	int i;
	float out[2];
	float a,b;

	if (firsttime) {
		sset(freq, tf0, tfN, strumq1);
		delayset(fbpitch, dq);
		firsttime = 0;
		}

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
makeFRET1()
{
	FRET1 *inst;

	inst = new FRET1();
	return inst;
}
