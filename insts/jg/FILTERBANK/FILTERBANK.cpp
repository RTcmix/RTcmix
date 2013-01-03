/* FILTERBANK - multi-band reson instrument
   This is like INPUTSIG from IIR, except that the parameters for each filter
   band are time-varying.

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier *
   p4 = ring-down duration
   p5 = input channel
   p6 = pan (in percent-to-left form: 0-1) *

   Followed by one or more (up to 60) filter band descriptions, given by
   triplets of
      a. center frequency (Hz or oct.pc) *
      b. bandwidth (percent of center frequency, from 0 to 1) *
      c. relative amplitude (0-1) *

   So the settings for the first band would occupy pfields 7-9, the second,
   pfields 10-12, and so on.

   * p3 (amplitude), p6 (pan) as well as the center frequency, bandwidth, and
   relative amplitude pfields for individual bands can receive dynamic updates
   from a table or real-time control source.  If you want to change the center
   frequency over time, use either Hz or linear octaves.  The latter requires
   converting to Hz with:  cf = makeconverter(cf, "cpsoct")

   The point of the ring-down duration parameter is to let you control
   how long the filter will sound after the input has stopped.  Too short
   a time, and the sound may be cut off prematurely.

   John Gibson <johgibso at gmail dot com>, 25 Feb 2007.
   Based on IIR and MULTEQ.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ugens.h>
#include <Ougens.h>
#include <Instrument.h>
#include <PField.h>
#include "FILTERBANK.h"
#include <rt.h>
#include <rtdefs.h>
#include <float.h>   // for FLT_MIN

#define FIRST_BAND_ARG	7
#define BAND_ARGS			3


FilterBand::FilterBand(float srate, float cf, float bw, float amp)
	: _cf(cf), _bw(bw), _amp(amp)
{
	_filt = new Oreson(srate, cf, bw, Oreson::kRMSResponse);
}


FilterBand::~FilterBand()
{
	delete _filt;
}


FILTERBANK::FILTERBANK()
	: branch(0), numbands(0), in(NULL), filt(NULL)
{
}


FILTERBANK::~FILTERBANK()
{
	delete [] in;
	for (int i = 0; i < numbands; i++)
		delete filt[i];
	delete [] filt;
}


int FILTERBANK::init(double p[], int n_args)
{
	nargs = n_args;
	float outskip = p[0];
	float inskip = p[1];
	float dur = p[2];
   float ringdur = p[4];
	inchan = int(p[5]);

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;
	insamps = int(dur * SR + 0.5);

	if (rtsetoutput(outskip, dur + ringdur, this) == -1)
		return DONT_SCHEDULE;

	if ((nargs - FIRST_BAND_ARG) % BAND_ARGS)
		return die("FILTERBANK", "For each band, need cf, bw, and amp.");

	numbands = (nargs - FIRST_BAND_ARG) / BAND_ARGS;
   filt = new FilterBand * [numbands];

	int band = 0;
	for (int i = FIRST_BAND_ARG; i < nargs; i += BAND_ARGS) {
		float cf = p[i] < 15.0 ? cpspch(p[i]) : p[i];
		float bw = p[i + 1];
		float amp = p[i + 2];
		filt[band] = new FilterBand(SR, cf, bw, amp);
		band++;
	}

	skip = int(SR / (float) resetval);

	return nSamps();
}


void FILTERBANK::doupdate()
{
	double p[nargs];
	update(p, nargs);

	amp = p[3];
	pan = p[6];

	int band = 0;
	for (int i = FIRST_BAND_ARG; i < nargs; i += BAND_ARGS) {
		float cf = p[i] < 15.0 ? cpspch(p[i]) : p[i];
		if (cf > SR * 0.5)
			cf = SR * 0.5;
		float bw = p[i + 1] * cf;
		if (bw <= 0.0)
			bw = FLT_MIN;
		float famp = p[i + 2];
		filt[band]->setparams(cf, bw, famp);
		band++;
	}
}


int FILTERBANK::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];
	return in ? 0 : -1;
}


int FILTERBANK::run()
{
	const int inchans = inputChannels();
	const int outchans = outputChannels();
	const int samps = framesToRun() * inchans;

	if (currentFrame() < insamps)
		rtgetin(in, this, samps);

	for (int i = 0; i < samps; i += inchans) {
		if (--branch <= 0) {
			doupdate();
			branch = skip;
		}

		float insig;
		if (currentFrame() < insamps)
			insig = in[i + inchan];
		else
			insig = 0.0f;

		float outsig = 0.0f;
		for (int n = 0; n < numbands; n++)
			outsig += filt[n]->next(insig);

		float out[2];
		out[0] = outsig * amp;
		if (outchans == 2) {
			out[1] = out[0] * (1.0 - pan);
			out[0] *= pan;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeFILTERBANK()
{
	FILTERBANK *inst;

	inst = new FILTERBANK();
	inst->set_bus_config("FILTERBANK");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("FILTERBANK", makeFILTERBANK);
}
#endif
