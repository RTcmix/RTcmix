/* WAVESHAPE -- a waveshaping instrument
 
   p0 = start time
   p1 = duration
   p2 = pitch (hz or oct.pc)
   p3 = index low point
   p4 = index high point
   p5 = amp
   p6 = stereo spread (0-1) <optional>

   function slot 1 is amp envelope
            slot 2 is waveform to be shaped (generally sine)
            slot 3 is the transfer function
            slot 4 is the index envelope
*/
#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "WAVESHAPE.h"
#include <rt.h>
#include <rtdefs.h>

#ifdef COMPATIBLE_FUNC_LOCS         /* set in makefile.conf */
  #define AMP_GEN_SLOT     2
  #define WAVE_GEN_SLOT    1
  #define XFER_GEN_SLOT    3
  #define INDEX_GEN_SLOT   4
#else
  #define AMP_GEN_SLOT     1        /* so that we can use setline instead */
  #define WAVE_GEN_SLOT    2
  #define XFER_GEN_SLOT    3
  #define INDEX_GEN_SLOT   4
#endif


WAVESHAPE::WAVESHAPE() : Instrument()
{
	// future setup here?
}

int WAVESHAPE::init(float p[], int n_args)
{
	nsamps = rtsetoutput(p[0], p[1], this);

	waveform = floc(WAVE_GEN_SLOT);
	if (waveform == NULL) {
		die("WAVESHAPE", "You need to store a waveform in function %d.",
								WAVE_GEN_SLOT);
		return(DONT_SCHEDULE);
	}
	lenwave = fsize(WAVE_GEN_SLOT);

	if (p[2] < 15.0) p[2] = cpspch(p[2]);
	si = p[2] * (float)(lenwave/SR);

	ampenv = floc(AMP_GEN_SLOT);
	if (ampenv) {
		int lenamp = fsize(AMP_GEN_SLOT);
		tableset(p[1], lenamp, amptabs);
	}
	else
		advise("WAVESHAPE", "Setting phrase curve to all 1's.");

	xfer = floc(XFER_GEN_SLOT);
	lenxfer = fsize(XFER_GEN_SLOT);

	indenv = floc(INDEX_GEN_SLOT);
	lenind = fsize(INDEX_GEN_SLOT);
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

	skip = (int)(SR/(float)resetval);       // how often to update amp curve

	return(nsamps);
}


int WAVESHAPE::run()
{
	int i;
	float out[2];
	float aamp,ampi=0.,val,val2,val3;
	float index=0.;
	int branch;

	Instrument::run();

	aamp = amp;            /* in case ampenv == NULL */

	branch = 0;
	for (i = 0; i < chunksamps; i++) {
		if (--branch < 0) {
			if (ampenv)
				aamp = table(cursamp, ampenv, amptabs) * amp;
			index = diff * tablei(cursamp,indenv,indtabs)+indbase;
			ampi = index ? aamp/index : 0.0;
			branch = skip;
			}

		val = oscili(1.0,si,waveform,lenwave,&phs);  /* wave */
		val2 = wshape(val*index,xfer,lenxfer); /* waveshape */
		/*dc blocking filter*/
		val3 = a1*z1;
		z1 = b1*z1+val2;
		val3 += a0*z1;
		val3 *= ampi;
		out[0] = val3;

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
makeWAVESHAPE()
{
	WAVESHAPE *inst;

	inst = new WAVESHAPE();
	inst->set_bus_config("WAVESHAPE");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("WAVESHAPE",makeWAVESHAPE);
}

