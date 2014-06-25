/* PULSE - process a pulse wave signal with an IIR filter bank

   First, call setup to configure the filter bank:

      setup(cf1, bw1, gain1, cf2, bw2, gain2, ...)

	Each filter has a center frequency (cf), bandwidth (bw) and gain control.
	Frequency can be in Hz or oct.pc.  Bandwidth is in Hz, or if negative,
	is a multiplier of the center frequency.  Gain is the amplitude of this
	filter relative to the other filters in the bank.  There can be as many
	as 64 filters in the bank.

	Then call PULSE:

      p0 = output start time
      p1 = duration
      p2 = amplitude
      p3 = pitch (Hz or oct.pc)
      p4 = pan (in percent-to-left form: 0-1) [optional, default is 0] 

   p2 (amplitude), p3 (pitch) and p4 (pan) can receive dynamic updates
   from a table or real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p2 amplitude multiplier, even if the latter is dynamic.

   When changing pitch dynamically, be aware of the implications of the
   dual-format pitch specification.  If the values drop below 15, then
   they will be interpreted as oct.pc.  Also, if you gliss from 8.00 down
   to 7.00, you will not get what you intend, because, for example,
   8.00 - .01 is 7.99, which is a very high pitch.
                                          rev. for v4.0 by JGG, 7/10/04
*/
#include <stdio.h>
#include <Ougens.h>
#include <ugens.h>
#include <Instrument.h>
#include <PField.h>
#include <Option.h>	// fastUpdate
#include "PULSE.h"
#include <rt.h>
#include <rtdefs.h>


PULSE::PULSE() : Instrument()
{
	branch = 0;
	nresons = 0;
}

PULSE::~PULSE()
{
	for (int i = 0; i < nresons; i++)
		delete resons[i];
}


// In fastUpdate mode, we skip doupdate() entirely, instead updating only amp,
// and only from a table.  The table can be a makegen or a PField table.  PField
// tables must be "flattened" using copytable if they are compound (e.g. passed
// through a PField filter or multiplied by a constant).  We use p[ampindex] as
// an amp multiplier, unless using a PField table, in which case there is no amp
// multiplier -- the p[ampindex] value is the first table value.   -JGG

void PULSE::initamp(float dur, double p[], int ampindex, int ampgenslot)
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

inline float pitch2si(float SR, float pitch)
{
	return pitch < 15.0 ? cpspch(pitch) * 512.0 / SR : pitch * 512.0 / SR;
}

int PULSE::init(double p[], int n_args)
{
	float outskip = p[0];
	float dur = p[1];
	float pitch = p[3];
	pan = p[4];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	initamp(dur, p, 2, 1);

	si = pitch2si(SR, pitch);
	phase = 512.0;

	float cf[MAXFILTER], bw[MAXFILTER], gain[MAXFILTER];
	nresons = get_iir_filter_specs(cf, bw, gain);
	if (nresons == 0)
		die("PULSE", "You must call setup() first to describe filters.");

	for (int i = 0; i < nresons; i++) {
		// NB: All the IIR insts used the RMS scale factor.
		resons[i] = new Oreson(SR, cf[i], bw[i], Oreson::kRMSResponse);
		resonamp[i] = gain[i];
	}

	return nSamps();
}

void PULSE::doupdate()
{
	double p[5];
	update(p, 5);
	amp = p[2];
	if (amptable)
		amp *= tablei(currentFrame(), amptable, amptabs);
	si = pitch2si(SR, p[3]);
	pan = p[4];
}

int PULSE::run()
{
	for (int i = 0; i < framesToRun(); i++)  {
		if (--branch <= 0) {
			if (fastUpdate) {
				if (amptable)
					amp = ampmult * tablei(currentFrame(), amptable, amptabs);
			}
			else
				doupdate();
			branch = getSkip();
		}

		float sig = mypulse(1.0f);

		float out[2];
		out[0] = 0.0f;
		for (int j = 0; j < nresons; j++)
			out[0] += resons[j]->next(sig) * resonamp[j];

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

Instrument *makePULSE()
{
	PULSE *inst;

	inst = new PULSE();
	inst->set_bus_config("PULSE");

	return inst;
}

