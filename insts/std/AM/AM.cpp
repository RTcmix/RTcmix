/* AM - amplitude modulation of input

   p0 = output start time
   p1 = input start time
   p2 = duration
   p3 = amplitude multiplier *
   p4 = modulation oscillator frequency (Hz) **
   p5 = input channel [optional, default is 0]
   p6 = pan (in percent-to-left form: 0-1) [optional; default is 0]
   p7 = reference to AM modulator wavetable [optional; if missing, use
        gen 2 ***, or default to internal sine wave] 

   p3 (amplitude), p4 (mod freq) and p6 (pan) can receive dynamic updates
   from a table or real-time control source.

   Note that you get either amplitude modulation or ring modulation,
   depending on whether the modulator waveform is unipolar or bipolar
   (unipolar = amp. mod., bipolar = ring mod.).

   To make a unipolar sine wave, you have to add a DC component to shift
   the sine wave out of the negative area.  For example, the following
   creates a sine wave that oscillates between 0 and 1:

      wave = maketable("wave3", 1000, 0,.5,0, 1,.5,0)

   ---

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

   ** For backwards compatibility, if p4 is zero (or its first value is
   zero, if p4 is dynamic), then an old-style gen table 3 must be present,
   and will override any changing values in p4.  However, the preferred
   way is to use either a constant or dynamic pfield for p4 instead of
   the makegen.
   
   *** If p7 is missing, you must use an old-style gen table 2 for the
   modulator waveform. [added default sine wave, BGG 1/2012]


   Author unknown (probably Brad Garton).
	[yes it was me -- BGG]
   Modulator frequency table and xtra comments added by John Gibson, 1/12/02.
   rev for v4, JGG, 7/22/04
	added default sine wavetable if gen slot or PField is NULL -- BGG 1/2/12
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Instrument.h>
#include <PField.h>
#include "AM.h"
#include <rt.h>
#include <rtdefs.h>


AM::AM() : Instrument()
{
	in = NULL;
	modosc = NULL;
	freqtable = NULL;
	wavetable = NULL;
	branch = 0;
	ownWavetable = false;
}

AM::~AM()
{
	delete [] in;
	delete modosc;
	if (ownWavetable)
		delete [] wavetable;
}

int AM::init(double p[], int n_args)
{
	float outskip = p[0];
	float inskip = p[1];
	float dur = p[2];
	inchan = (int) p[5];

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;	// no input
	if (inchan >= inputChannels())
		return die("AM", "You asked for channel %d of a %d-channel file.",
														inchan, inputChannels());
	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() > 2)
		return die("AM", "Can't handle more than 2 output channels.");

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, dur, amplen, amptabs);
	}

	int tablelen = 0;
	if (n_args > 7) {      // handle table coming in as optional p7 TablePField
		wavetable = (double *) getPFieldTable(7, &tablelen);
	}
	if (wavetable == NULL) { // use old gen slot
		wavetable = floc(2);
		if (wavetable)
			tablelen = fsize(2);
		else { // use default sine wave
			rtcmix_advise("AM", "No modulator wavetable specified, so using sine wave.");
			tablelen = 1024;
			wavetable = new double [tablelen];
			ownWavetable = true;
			const double twopi = M_PI * 2.0;
			for (int i = 0; i < tablelen; i++)
				wavetable[i] = sin(twopi * ((double) i / tablelen));
		}
	}

	modfreq = p[4];
	modosc = new Ooscili(SR, modfreq, wavetable, tablelen);

	if (modfreq == 0.0) {
		freqtable = floc(3);
		if (freqtable) {
			int len = fsize(3);
      	tableset(SR, dur, len, freqtabs);
		}
		else
			return die("AM", "If p4 is zero, old-style gen table 3 must "
									"contain modulator frequency curve.");
	}

	skip = (int) (SR / (float) resetval);

	return nSamps();
}

int AM::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];
	return in ? 0 : -1;
}

int AM::run()
{
	const int samps = framesToRun() * inputChannels();

	rtgetin(in, this, samps);

	for (int i = 0; i < samps; i += inputChannels())  {
		if (--branch <= 0) {
			double p[7];
			update(p, 7, kAmp | kFreq | kPan);
			amp = p[3];
			if (amptable)
				amp *= tablei(currentFrame(), amptable, amptabs);
			if (freqtable)
				modfreq = tablei(currentFrame(), freqtable, freqtabs);
			else
				modfreq = p[4];
			modosc->setfreq(modfreq);
			spread = p[6];
			branch = skip;
		}

		float out[2];
		out[0] = in[i + inchan] * modosc->next() * amp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - spread);
			out[0] *= spread;
		}

		rtaddout(out);
		increment();
	}
	return framesToRun();
}

Instrument *makeAM()
{
	AM *inst;

	inst = new AM();
	inst->set_bus_config("AM");

	return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("AM",makeAM);
}
#endif
