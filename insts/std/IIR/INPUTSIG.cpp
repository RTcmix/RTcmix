/* INPUTSIG - process a mono input with an IIR filter bank

   First, call setup to configure the filter bank:

      setup(cf1, bw1, gain1, cf2, bw2, gain2, ...)

	Each filter has a center frequency (cf), bandwidth (bw) and gain control.
	Frequency can be in Hz or oct.pc.  Bandwidth is in Hz, or if negative,
	is a multiplier of the center frequency.  Gain is the amplitude of this
	filter relative to the other filters in the bank.  There can be as many
	as 64 filters in the bank.

	Then call INPUTSIG:

      p0 = output start time
      p1 = input start time
      p2 = duration
      p3 = amplitude multiplier
      p4 = input channel [optional, default is 0]
      p5 = pan (in percent-to-left form: 0-1) [optional, default is 0] 

   p3 (amplitude) and p5 (pan) can receive dynamic updates from a table
   or real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

                                          rev. for v4.0 by JGG, 7/10/04
*/
#include <stdio.h>
#include <ugens.h>
#include <Ougens.h>
#include <Instrument.h>
#include <PField.h>
#include <Option.h>	// fastUpdate
#include "INPUTSIG.h"
#include <rt.h>
#include <rtdefs.h>


INPUTSIG::INPUTSIG() : Instrument()
{
	in = NULL;
	branch = 0;
	nresons = 0;
}

INPUTSIG::~INPUTSIG()
{
	delete [] in;
	for (int i = 0; i < nresons; i++)
		delete resons[i];
}


// In fastUpdate mode, we skip doupdate() entirely, instead updating only amp,
// and only from a table.  The table can be a makegen or a PField table.  PField
// tables must be "flattened" using copytable if they are compound (e.g. passed
// through a PField filter or multiplied by a constant).  We use p[ampindex] as
// an amp multiplier, unless using a PField table, in which case there is no amp
// multiplier -- the p[ampindex] value is the first table value.   -JGG

void INPUTSIG::initamp(float dur, double p[], int ampindex, int ampgenslot)
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

int INPUTSIG::init(double p[], int n_args)
{
	float outskip = p[0];
	float inskip = p[1];
	float dur = p[2];
	inchan = n_args > 4 ? (int) p[4] : 0;
	pan = p[5];

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;	// no input

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	initamp(dur, p, 3, 1);

	float cf[MAXFILTER], bw[MAXFILTER], gain[MAXFILTER];
	nresons = get_iir_filter_specs(cf, bw, gain);
	if (nresons == 0)
		die("INPUTSIG", "You must call setup() first to describe filters.");

	for (int i = 0; i < nresons; i++) {
		// NB: All the IIR insts used the RMS scale factor.
		resons[i] = new Oreson(SR, cf[i], bw[i], Oreson::kRMSResponse);
		resonamp[i] = gain[i];
	}

	if (inchan >= inputChannels())
		return die("INPUTSIG", "You asked for channel %d of a %d-channel input.",
														inchan, inputChannels());

	return nSamps();
}

int INPUTSIG::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];
	return in ? 0 : -1;
}

void INPUTSIG::doupdate()
{
	double p[6];
	update(p, 6, kAmp | kPan);
	amp = p[3];
	if (amptable)
		amp *= tablei(currentFrame(), amptable, amptabs);
	pan = p[5];
}

int INPUTSIG::run()
{
	int samps = framesToRun() * inputChannels();

	rtgetin(in, this, samps);

	for (int i = 0; i < samps; i += inputChannels())  {
		if (--branch <= 0) {
			if (fastUpdate) {
				if (amptable)
					amp = ampmult * tablei(currentFrame(), amptable, amptabs);
			}
			else
				doupdate();
			branch = getSkip();
		}

		float insig = in[i + inchan];
		float out[2];
		out[0] = 0.0f;
		for (int j = 0; j < nresons; j++)
			out[0] += resons[j]->next(insig) * resonamp[j];

		out[0] *= amp;
		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0f - pan);
			out[0] *= pan;
		}

		rtaddout(out);
		increment();
	}
	return framesToRun();
}


Instrument *makeINPUTSIG()
{
	INPUTSIG *inst;

	inst = new INPUTSIG();
	inst->set_bus_config("INPUTSIG");

	return inst;
}

