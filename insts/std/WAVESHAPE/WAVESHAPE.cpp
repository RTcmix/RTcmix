#include <iostream.h>
#include "../../sys/mixerr.h"
#include "../../rtstuff/Instrument.h"
#include "WAVESHAPE.h"
#include "../../rtstuff/rt.h"
#include "../../rtstuff/rtdefs.h"


extern "C" {
	#include "../../H/ugens.h"
}

WAVESHAPE::WAVESHAPE() : Instrument()
{
	// future setup here?
}

int WAVESHAPE::init(float p[], short n_args)
{
// p0 = start; p1 = dur; p2 = pitch (hz or oct.pc); 
// p3 = index low point; p4 = index  high point; p5 = amp
// p6 = stereo spread (0-1) <optional>
// function slot 1 is waveform to be shaped (generally sine)
//    slot 2 is amp envelope
//    slot 3 is the transfer function
//    slot 4 is the index envelope

	nsamps = rtsetoutput(p[0], p[1], this);

	waveform = floc(1);    /* function 1 is waveform */
	lenwave = fsize(1);
	if (p[2] < 15.0) p[2] = cpspch(p[2]);
	si = p[2] * (float)(lenwave/SR);

	ampenv = floc(2);
	lenamp = fsize(2);
	tableset(p[1], lenamp, amptabs);

	xfer = floc(3);
	lenxfer = fsize(3);

	indenv = floc(4);  /* function 4 is index guide */
	lenind = fsize(4);
	tableset(p[1],lenind,indtabs);

	diff = p[4] - p[3];
	indbase = p[3];

	/*initialize dc blocking filter*/
	c=PI*(float)(p[2]/2./SR);  /*cutoff frequency at pitch/2 */
	a0=1./(1.+c);
	a1= -a0;
	b1=a0*(1-c);
	z1=0;

	phs = 0.0;
	amp = p[5];
	spread = p[6];

	return(nsamps);
}


int WAVESHAPE::run()
{
	int i;
	float out[2];
	float aamp,ampi,val,val2,val3;
	float index;
	int branch;

	branch = 0;
	for (i = 0; i < chunksamps; i++) {
		if (--branch < 0) {
			aamp = table(cursamp, ampenv, amptabs) * amp;
			index = diff * tablei(cursamp,indenv,indtabs)+indbase;
			ampi = aamp/index;
			branch = 0;
			}

		val = oscili(1.0,si,waveform,lenwave,&phs);  /* wave */
		val2 = wshape(val*index,xfer,lenxfer); /* waveshape */
		/*dc blocking filter*/
		val3 = a1*z1;
		z1 = b1*z1+val2;
		val3 += a0*z1;
		val3 *= ampi;
		out[0] = val3;

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
makeWAVESHAPE()
{
	WAVESHAPE *inst;

	inst = new WAVESHAPE();
	return inst;
}

void
rtprofile()
{
	RT_INTRO("WAVESHAPE",makeWAVESHAPE);
}

