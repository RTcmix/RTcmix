/* WAVETABLE -- simple wavetable oscillator instrument
 
   p0 = start time
   p1 = duration
   p2 = amp *
   p3 = frequency (Hz or oct.pc **)
   p4 = pan (in percent-to-left form: 0-1) [optional; default is 0]
   p5 = reference to wavetable [optional; if missing, must use gen 2 ***]

   p2 (amplitude), p3 (freq) and p4 (pan) can receive dynamic updates
   from a table or real-time control source.

   * If an old-style gen table 1 is present, its values will be multiplied
   by the p2 amplitude multiplier, even if the latter is dynamic.

   ** oct.pc format generally will not work as you expect for p3 (osc freq)
   if the pfield changes dynamically.  Use Hz instead in that case.

   *** If p5 is missing, you must use an old-style gen table 2 for the
   oscillator waveform.

                                                rev for v4, JGG, 7/12/04
*/

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Instrument.h>
#include <PField.h>
#include <Option.h>	// fastUpdate
#include "WAVETABLE.h"
#include <rt.h>
#include <rtdefs.h>

#define AMP_GEN_SLOT     1
#define WAVET_GEN_SLOT   2


WAVETABLE::WAVETABLE() : Instrument()
{
	branch = 0;
}

WAVETABLE::~WAVETABLE()
{
	delete osc;
}

// In fastUpdate mode, we skip doupdate() entirely, instead updating only amp,
// and only from a table.  The table can be a makegen or a PField table.  PField
// tables must be "flattened" using copytable if they are compound (e.g. passed
// through a PField filter or multiplied by a constant).  We use p[ampindex] as
// an amp multiplier, unless using a PField table, in which case there is no amp
// multiplier -- the p[ampindex] value is the first table value.   -JGG

void WAVETABLE::initamp(float dur, double p[], int ampindex, int ampgenslot)
{
	fastUpdate = Option::fastUpdate();
	if (fastUpdate) {
		// Prefer PField table, otherwise makegen
		int tablen = 0;
		amptable = (double *) getPFieldTable(ampindex, &tablen);
		if (amptable)
			ampmult = 1.0f;
		else {
			ampmult = p[ampindex];
			amptable = floc(ampgenslot);
			if (amptable)
				tablen = fsize(ampgenslot);
		}
		if (amptable)
			tableset(SR, dur, tablen, amptabs);
		else
			amp = ampmult;
	}
	else {
		// NB: ampmult never used, first amp set in doupdate
		amptable = floc(ampgenslot);
		if (amptable) {
			int tablen = fsize(ampgenslot);
			tableset(SR, dur, tablen, amptabs);
		}
	}
}

int WAVETABLE::init(double p[], int n_args)
{
	float outskip = p[0];
	float dur = p[1];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() > 2)
		return die("WAVETABLE", "Can't handle more than 2 output channels.");

	initamp(dur, p, 2, AMP_GEN_SLOT);

	freqraw = p[3];
	float freq;
	if (freqraw < 15.0)
		freq = cpspch(freqraw);
	else
		freq = freqraw;

	spread = p[4];

	wavetable = NULL;
	int tablelen = 0;
	if (n_args > 5)		// handle table coming in as optional p5 TablePField
		wavetable = (double *) getPFieldTable(5, &tablelen);
	if (wavetable == NULL) {
		wavetable = floc(WAVET_GEN_SLOT);
		if (wavetable == NULL)
			return die("WAVETABLE", "Either use the wavetable pfield (p5) or make "
                    "an old-style gen function in slot %d.", WAVET_GEN_SLOT);
		tablelen = fsize(WAVET_GEN_SLOT);
	}
	osc = new Ooscili(SR, freq, wavetable, tablelen);

	return nSamps();
}

void WAVETABLE::doupdate()
{
	double p[6];
	update(p, 6, 1 << 2 | 1 << 3 | 1 << 4 | 1 << 5);

	amp = p[2];
	if (amptable)
		amp *= tablei(currentFrame(), amptable, amptabs);

	if (p[3] != freqraw) {
		freqraw = p[3];
		float freq = (freqraw < 15.0) ? cpspch(freqraw) : freqraw;
		osc->setfreq(freq);
	}

	spread = p[4];
}

int WAVETABLE::run()
{
	const int nframes = framesToRun();
	for (int i = 0; i < nframes; i++) {
		if (--branch <= 0) {
			if (fastUpdate) {
				if (amptable)
					amp = ampmult * tablei(currentFrame(), amptable, amptabs);
			}
			else
				doupdate();
			branch = getSkip();
		}

		float out[2];
		out[0] = osc->next() * amp;

		if (outputChannels() == 2) {
			out[1] = (1.0 - spread) * out[0];
			out[0] *= spread;
		}

		rtaddout(out);
		increment();
	}
	return framesToRun();
}


Instrument *makeWAVETABLE()
{
	WAVETABLE *inst;
	inst = new WAVETABLE();
	inst->set_bus_config("WAVETABLE");
	return inst;
}

void rtprofile()
{
	RT_INTRO("WAVETABLE", makeWAVETABLE);
}

