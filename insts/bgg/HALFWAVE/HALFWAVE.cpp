/* HALFWAVE - specify two 1/2-waveforms, modulate the mid-crossover point

	HALFWAVE takes two 1/2-waveform tables and changes the proportion
	of each in the written waveform by dilating or compressing each one

	p0 = output start time
	p1 = duration
	*p2 = pitch (Hz or oct.pc)
	*p3 = amplitude
	p4 = first half wavetable
	p5 = second half wavetable
	*p6 = wavetable mid-crossover point [0-1]
	*p7 = pan [optional; default is 0]

	* p-fields marked with an asterisk can receive dynamic updates
	from a table or real-time control source

	BGG, 7/2007
*/

#include <stdlib.h>
#include <Instrument.h>
#include <ugens.h>
#include <Ougens.h>
#include "HALFWAVE.h"
#include <rt.h>
#include <rtdefs.h>

HALFWAVE::HALFWAVE() : Instrument()
{
}

HALFWAVE::~HALFWAVE()
{
	delete theOscils[0];
	delete theOscils[1];
}

int HALFWAVE::init(double p[], int n_args)
{
	float freq;
	double *oscwave;

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() > 2)
		return die("HALFWAVE", "Can't handle more than 2 output channels.");

	if (p[2] < 15.0)
		freq = cpspch(p[2]);
	else
		freq = p[2];

	oscwave = (double *) getPFieldTable(4, wavelens);
	theOscils[0] = new Ooscili(SR, freq*2.0, oscwave, wavelens[0]);

	oscwave = (double *) getPFieldTable(5, wavelens+1);
	theOscils[1] = new Ooscili(SR, freq*2.0, oscwave, wavelens[1]);

	endpoint = (1.0/freq) * SR;	// number of samps (fractional) in 1 cycle
	divpoint = endpoint * p[6];

	amp = p[3];
	oscnum = 0;
	branch = 0;
	sample_count = 0.0;
	spread = p[7];

	return nSamps();
}

void HALFWAVE::doupdate()
{
	double p[8];
	float freq;

	update(p, 8);

	if (p[2] < 15.0)
		freq = cpspch(p[2]);
	else
		freq = p[2];
	endpoint = (1.0/freq) * SR;	// number of samps (fractional) in 1 cycle

	divpoint = endpoint * p[6];
	if (divpoint == 0.0) divpoint = 0.0001; // just in case...
	theOscils[0]->setfreq(1.0/p[6] * freq);
	if (divpoint == 1.0) divpoint = 0.9999; // just in case...
	theOscils[1]->setfreq(1.0/(1.0-p[6]) * freq);

	amp = p[3];
	spread = p[7];
}

int HALFWAVE::run()
{
	int i;
	float out[2];
	double pval;
	
	for (i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			doupdate();
			branch = getSkip();
		}

		out[0] = theOscils[oscnum]->next() * amp;

		if (outputChannels() > 1) {
			out[1] = out[0] * (1.0 - spread);
			out[0] *= spread;
		};

		rtaddout(out);

		sample_count += 1.0;
		if (sample_count > endpoint) {
			oscnum = 0;
			pval = (sample_count - endpoint)/(double)wavelens[1] * (double)wavelens[0];
			theOscils[oscnum]->setphase(pval);
			sample_count = sample_count - endpoint;
		} else if ( (sample_count > divpoint) && (oscnum == 0) ) {
			oscnum = 1;
			pval = (sample_count - divpoint)/(double)wavelens[0] * (double)wavelens[1];
			theOscils[oscnum]->setphase(pval);
		}

		increment();
	}
	return i;
}

Instrument*
makeHALFWAVE()
{
	HALFWAVE *inst;
	inst = new HALFWAVE();
	inst->set_bus_config("HALFWAVE");
	return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("HALFWAVE",makeHALFWAVE);
}
#endif
