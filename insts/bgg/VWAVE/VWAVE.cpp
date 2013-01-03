/* VWAVE - "vector" wavetable synthesis with arbitrary # of wavetable

	p0 = output start tim
	p1 = duration
	*p2 = pitch (Hz or oct.pc)
	*p3 = amplitude
	*p4 = wavetable vector guide [0-1]
	*p5 = pan [0-1]
	p6... pn = wavetables

	* p-fields marked with an asterisk can receive dynamic updates
	from a table or real-time control source

	BGG, 7/2007
*/

#include <stdlib.h>
#include <Instrument.h>
#include <ugens.h>
#include <Ougens.h>
#include "VWAVE.h"
#include <rt.h>
#include <rtdefs.h>

VWAVE::VWAVE() : Instrument()
{
}

VWAVE::~VWAVE()
{
	int i;

	for (i = 0; i < ndivs; i++)
		delete theOscils[i];

	delete [] theOscils;     
}

int VWAVE::init(double p[], int n_args)
{
	float freq;
	int i;
	int wavelen;
	double *oscwave;

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() > 2)
		return die("VWAVE", "Can't handle more than 2 output channels.");

	if (p[2] < 15.0)
		freq = cpspch(p[2]);
	else
		freq = p[2];

	divpoints = new double[n_args-5]; // additional point at 1.0
	theOscils = new Ooscili *[n_args-6];
	for (i = 6; i < n_args; i++) {
		oscwave = (double *) getPFieldTable(i, &wavelen);
		theOscils[i-6] = new Ooscili(SR, freq, oscwave, wavelen);
		if ((i-6) == 0) divpoints[0] = 0.0;
		else divpoints[i-6] = (double)(i-6)/(double)(n_args-6);
	}
	divpoints[i-6] = 1.0;
	ndivs = n_args-6;

	amp = p[3];
	vecdex = p[4];
	spread = p[5];
	branch = 0;

	return nSamps();
}

void VWAVE::doupdate()
{
	int i;
	double p[6];
	float freq;

	update(p, 6);

	if (p[2] < 15.0)
		freq = cpspch(p[2]);
	else
		freq = p[2];

	for (i = 0; i < ndivs; i++)
			theOscils[i]->setfreq(freq);

	amp = p[3];
	vecdex = p[4];
	spread = p[5];
}

int VWAVE::run()
{
	int i,j;
	float out[2];
	int dcount;
	double famp;
	double phasemult;
	int osc_zero_used;

	for (i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			doupdate();
			branch = getSkip();
		}

		dcount = ndivs;
		osc_zero_used = 0;
		while (--dcount >= 0) {
			if (vecdex >= divpoints[dcount]) {
				famp = (vecdex-divpoints[dcount])/(divpoints[dcount+1]-divpoints[dcount]);
				if (dcount > 0) {
					out[0] = theOscils[dcount]->next() * amp * famp;
					out[0] += theOscils[dcount-1]->next() * amp * (1.0-famp);
					if (dcount-1 == 0) osc_zero_used = 1;
				} else {
					out[0] = theOscils[dcount]->next() * amp;
					osc_zero_used = 1;
				}
				dcount = 0;
			}
		}

		// this is to keep all theOscils in phase, otherwise weird amp effects
		if (osc_zero_used == 0) theOscils[0]->next();
		phasemult = theOscils[0]->getphase()/theOscils[0]->getlength();
		for (j = 0; j < ndivs; j++)
			theOscils[j]->setphase(phasemult * theOscils[j]->getlength());

		if (outputChannels() > 1) {
			out[1] = out[0] * (1.0 - spread);
			out[0] *= spread;
		};

		rtaddout(out);
		increment();
	}
	return i;
}

Instrument*
makeVWAVE()
{
	VWAVE *inst;
	inst = new VWAVE();
	inst->set_bus_config("VWAVE");
	return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("VWAVE",makeVWAVE);
}
#endif
