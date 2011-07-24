/*	Mix inputs to outputs with global amplitude control.

		p0 = output start time
		p1 = input start time
		p2 = duration (-endtime)
		p3 = amplitude multiplier
		p4-n = channel mix maxtrix

	p3 (amplitude) can receive dynamic updates from a table or real-time
	control source.

	If an old-style gen table 1 is present, its values will be multiplied
	by the p3 amplitude multiplier, even if the latter is dynamic.

	rev for v4, JGG, 7/9/04
*/
#include <unistd.h>
#include <stdio.h>
#include <ugens.h>
#include <Instrument.h>
#include <PField.h>
#include <Option.h>	// for fastUpdate
#include <rt.h>
#include <rtdefs.h>
#include "MIX.h"


MIX::MIX() : Instrument(), in(NULL)
{
	branch = 0;
}

MIX::~MIX()
{
	delete [] in;
}

// In fastUpdate mode, we skip doupdate() entirely, instead updating only amp,
// and only from a table.  The table can be a makegen or a PField table.  PField
// tables must be "flattened" using copytable if they are compound (e.g. passed
// through a PField filter or multiplied by a constant).  We use p[ampindex] as
// an amp multiplier, unless using a PField table, in which case there is no amp
// multiplier -- the p[ampindex] value is the first table value.   -JGG

void MIX::initamp(float dur, double p[], int ampindex, int ampgenslot)
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

int MIX::init(double p[], int n_args)
{
	const float outskip = p[0];
	const float inskip = p[1];
	float dur = p[2];
	if (dur < 0.0)
		dur = -dur - inskip;

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;	// no input

	for (int i = 0; i < inputChannels(); i++) {
		outchan[i] = (int) p[i + 4];
		if (outchan[i] + 1 > outputChannels())
			return die("MIX",
						"You wanted output channel %d, but have only specified "
						"%d output channels", outchan[i], outputChannels());
	}

	initamp(dur, p, 3, 1);

	return nSamps();
}


int MIX::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];
	return in ? 0 : -1;
}


int MIX::run()
{
	const int inchans = inputChannels();
	const int samps = framesToRun() * inchans;

	rtgetin(in, this, samps);

	for (int i = 0; i < samps; i += inchans)  {
		if (--branch <= 0) {
			if (fastUpdate) {
				if (amptable)
					amp = ampmult * tablei(currentFrame(), amptable, amptabs);
			}
			else {
				double p[4];
				update(p, 4, 1 << 3);
				amp = p[3];
				if (amptable)		// legacy makegen
					amp *= tablei(currentFrame(), amptable, amptabs);
			}
			branch = getSkip();
		}

		float out[MAXBUS];
		const int ochans = outputChannels();
		for (int j = 0; j < ochans; j++) {
			out[j] = 0.0;
			for (int k = 0; k < inchans; k++) {
				if (outchan[k] == j)
					out[j] += in[i+k] * amp;
			}
		}

		rtaddout(out);
		increment();
	}
	return framesToRun();
}


Instrument *makeMIX()
{
	MIX *inst;

	inst = new MIX();
	inst->set_bus_config("MIX");

	return inst;
}


void rtprofile()
{
   RT_INTRO("MIX",makeMIX);
}
