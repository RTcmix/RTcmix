/* SYNC - 'hard' sync oscillator synthesis instrument

	SYNC periodically resets the writing of an oscillating waveform,
	'hard' syncing the writing oscillator to the resetting "oscillator"
	(the reset rate is the resetting "oscillator" frequency).  Sweeping
	the frequency of the writing oscillator (p4) will produce differing
	amounts of discontinuity in the resulting waveform

	p0 = output start time
	p1 = duration
	*p2 = pitch (reset frequency) (Hz or oct.pc)
	*p3 = amplitude
	*p4 = oscillator writing frequency
	p5 = oscillator writing wavetable
	*p6 = pan [optional; default is 0]

	* p-fields marked with an asterisk can receive dynamic updates
	from a table or real-time control source

	BGG, 7/2007
*/

#include <stdlib.h>
#include <Instrument.h>
#include <ugens.h>
#include <Ougens.h>
#include "SYNC.h"
#include <rt.h>
#include <rtdefs.h>

SYNC::SYNC() : Instrument()
{
}

SYNC::~SYNC()
{
	delete theOscil;
}

int SYNC::init(double p[], int n_args)
{
	int len;
	float freq;
	double *oscwave;

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() > 2)
		return die("SYNC", "Can't handle more than 2 output channels.");

	oscwave = (double *) getPFieldTable(5, &len);
	theOscil = new Ooscili(SR, p[4], oscwave, len);

	if (p[2] < 15.0)
		freq = cpspch(p[2]);
	else
		freq = p[2];
	reset_samps = (1.0/freq) * SR; 		// this is the pitch we hear
	sample_count = 0.0;

	branch = 0;

	return nSamps();
}

void SYNC::doupdate()
{
	double p[7];
	float freq;

	update(p, 7);

	if (p[2] < 15.0)
		freq = cpspch(p[2]);
	else
		freq = p[2];
	reset_samps = (1.0/freq) * SR; 		// this is the pitch we hear

	amp = p[3];
	theOscil->setfreq(p[4]);
	spread = p[6];
}

int SYNC::run()
{
	int i;
	float out[2];
	
	for (i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			doupdate();
			branch = getSkip();
		}

		out[0] = theOscil->next() * amp;

		if (outputChannels() > 1) {
			out[1] = out[0] * (1.0 - spread);
			out[0] *= spread;
		};

		rtaddout(out);

		sample_count += 1.0;
		// when sample_count reachs the reset point, we reset both
		// the phase and the frequency of theOscil
		if (sample_count > reset_samps)
		{
			theOscil->setphase(0.0);
			sample_count = sample_count - reset_samps;
		}

		increment();
	}
	return i;
}

Instrument*
makeSYNC()
{
	SYNC *inst;
	inst = new SYNC();
	inst->set_bus_config("SYNC");
	return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("SYNC",makeSYNC);
}
#endif
