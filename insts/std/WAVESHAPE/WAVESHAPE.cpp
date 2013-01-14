/* WAVESHAPE -- waveshaping synthesis instrument
 
   p0 = start time
   p1 = duration
   p2 = frequency (Hz or oct.pc)
   p3 = minimum distortion index
   p4 = maximum distortion index
   p5 = amp *
   p6 = pan (in percent-to-left form: 0-1) [optional; default is 0]
   p7 = reference to oscillator waveform table [optional; if missing,
        must use gen 2] **
   p8 = reference to waveshaping tranfer function table [optional; if missing,
        must use gen 3] ***
   p9 = index guide [optional; if missing, must use gen 4] ****
   p10 = amp normalization [optional; default is on (1)]

   p2 (freq), p3 (min index), p4 (max index), p5 (amp), p6 (pan) and
   p9 (index) can receive dynamic updates from a table or real-time
   control source.

   NOTE: The amp normalization in this instrument can cause clicks at
   the beginning and ending of notes.  Passing zero for p10 turns it off.

   ----

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by p5 (amplitude), even if the latter is dynamic.

   ** If p7 is missing, you must use an old-style gen table 2 for the
   oscillator waveform.

   *** If p8 is missing, you must use an old-style gen table 3 for the
   waveshaping transfer function.

   **** If p9 is missing, you must use an old-style gen table 4 for the
   distortion index curve.

                                                rev for v4, JGG, 7/22/04
*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ugens.h>
#include <Instrument.h>
#include <PField.h>
#include "WAVESHAPE.h"
#include <rt.h>
#include <rtdefs.h>

#define AMP_GEN_SLOT     1
#define WAVE_GEN_SLOT    2
#define XFER_GEN_SLOT    3
#define INDEX_GEN_SLOT   4


WAVESHAPE::WAVESHAPE() : Instrument()
{
	osc = NULL;
	branch = 0;
}

WAVESHAPE::~WAVESHAPE()
{
	delete osc;
}

int WAVESHAPE::init(double p[], int n_args)
{
	nargs = n_args;
	float outskip = p[0];
	float dur = p[1];
	rawfreq = p[2];
	doampnorm = n_args > 10 ? (bool) p[10] : true;

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() > 2)
		return die("WAVESHAPE", "Can't handle more than 2 output channels.");

	waveform = NULL;
	int tablelen = 0;
	if (n_args > 7) {		// handle table coming in as optional p7 TablePField
		waveform = (double *) getPFieldTable(7, &tablelen);
	}
	if (waveform == NULL) {
		waveform = floc(WAVE_GEN_SLOT);
		if (waveform == NULL)
			return die("WAVESHAPE", "Either use the wavetable pfield (p7) or make "
						"an old-style gen function in slot %d.", WAVE_GEN_SLOT);
		tablelen = fsize(WAVE_GEN_SLOT);
	}

	float freq = rawfreq;
	if (rawfreq < 15.0)
		freq = cpspch(rawfreq);

	osc = new Ooscili(SR, freq, waveform, tablelen);

	xferfunc = NULL;
	lenxfer = 0;
	if (n_args > 8) {		// handle table coming in as optional p8 TablePField
		xferfunc = (double *) getPFieldTable(8, &lenxfer);
	}
	if (xferfunc == NULL) {
		xferfunc = floc(XFER_GEN_SLOT);
		if (xferfunc == NULL)
			return die("WAVESHAPE", "Either use the transfer function pfield "
						"(p8) or make an old-style gen function in slot %d.",
						XFER_GEN_SLOT);
		lenxfer = fsize(XFER_GEN_SLOT);
	}

	indenv = NULL;
	if (n_args < 10) {	// no p9 guide PField, so must use gen table
		indenv = floc(INDEX_GEN_SLOT);
		if (indenv == NULL)
			return die("WAVESHAPE", "Either use the index pfield (p9) or make "
						"an old-style gen function in slot %d.", INDEX_GEN_SLOT);
		lenind = fsize(INDEX_GEN_SLOT);
		tableset(SR, dur, lenind, indtabs);
	}

	ampenv = floc(AMP_GEN_SLOT);
	if (ampenv) {
		int lenamp = fsize(AMP_GEN_SLOT);
		tableset(SR, dur, lenamp, amptabs);
	}

	setDCBlocker(freq, true);		// initialize dc blocking filter

	skip = (int) (SR / (float) resetval);

	return nSamps();
}

void WAVESHAPE::setDCBlocker(float freq, bool init)
{
	float c = M_PI * (freq / 2.0 / SR);  // cutoff frequency at freq/2
	a0 = 1.0 / (1.0 + c);
	a1 = -a0;
	b1 = a0 * (1.0 - c);
	if (init)
		z1 = 0.0;
}

void WAVESHAPE::doupdate()
{
	double p[10];
	update(p, 10, kFreq | kMinIndex | kMaxIndex | kAmp | kPan | kIndex);

	if (rawfreq != p[2]) {
		rawfreq = p[2];
		float freq = rawfreq;
		if (rawfreq < 15.0)
			freq = cpspch(rawfreq);
		osc->setfreq(freq);
		setDCBlocker(freq, false);
	}

	float min_index = p[3];
	float max_index = p[4];
	if (max_index < min_index)
		max_index = min_index;

	float rawamp = p[5];
	if (ampenv)
		rawamp *= tablei(currentFrame(), ampenv, amptabs);

	spread = p[6];

	float rawindex;
	if (nargs > 9)
		rawindex = p[9];
	else
		rawindex = tablei(currentFrame(), indenv, indtabs);
	index = min_index + ((max_index - min_index) * rawindex);

	if (doampnorm)
		amp = index ? rawamp / index : 0.0;
	else
		amp = rawamp;
}

int WAVESHAPE::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			doupdate();
			branch = skip;
		}

		float sig = osc->next();
		float wsig = wshape(sig * index, xferfunc, lenxfer);

		// dc blocking filter
		float osig = a1 * z1;
		z1 = b1 * z1 + wsig;
		osig += a0 * z1;

		float out[2];
		out[0] = osig * amp;

		if (outputChannels() == 2) {
			out[1] = (1.0 - spread) * out[0];
			out[0] *= spread;
		}

		rtaddout(out);
		increment();
	}
	return framesToRun();
}


Instrument *makeWAVESHAPE()
{
	WAVESHAPE *inst;

	inst = new WAVESHAPE();
	inst->set_bus_config("WAVESHAPE");

	return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("WAVESHAPE",makeWAVESHAPE);
}
#endif
