/* MBANDEDWG - the "BandedWG" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = strike position (0.0-1.0)
   p5 = pluck flag (0: no pluck, 1: pluck)
   p6 = max velocity (0.0-1.0, I think...)
   p7 = preset #
         - Uniform Bar = 0
         - Tuned Bar = 1
         - Glass Harmonica = 2
         - Tibetan Bowl = 3
   p8 = bow pressure (0.0-1.0) 0.0 == strike only
   p9 = mode resonance (0.0-1.0) 0.99 == normal strike
   p10 = integration constant (0.0-1.0) 0.0 == normal?
   p11 = percent of signal to left output channel [optional, default is .5]
   p12 = velocity table [optional, default is makegen(2, ...) or all 1's]
      interacts with p8

   PField updating:
      p2 (amplitude)
         (or assumes function table 1 is amplitude curve for the note.
         If no setline or function table 1, uses flat amplitude curve.)
      p3 (frequency)
      p8 (bow pressure)
      p9 (mode resonance)
      p10 (integration constant)
      p11 (pan)
      p12 (velocity table)
*/

#include <Stk.h>
#include <BandedWG.h> // from the stk library

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include <math.h>
#include <float.h>
#include "MBANDEDWG.h"         /* declarations for this instrument class */
#include <rt.h>
#include <rtdefs.h>



MBANDEDWG :: MBANDEDWG()
	: branch(0), theBar(NULL)
{
}

MBANDEDWG :: ~MBANDEDWG()
{
	delete theBar;
}

int MBANDEDWG :: init(double p[], int n_args)
{
	nargs = n_args;
	Stk::setSampleRate(SR);

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	amptable = floc(1);
	if (amptable) // the amp array has been created using makegen
		theEnv = new Ooscili(SR, 1.0/p[1], 1);

	veltable = floc(2);
	if (veltable) { // the velocity array has been created in the score
		theVeloc = new Ooscili(SR, 1.0/p[1], 2);
	} else {
		if (n_args < 13) {
			velarray[0] = velarray[1] = 1.0;
			rtcmix_advise("MBANDEDWG", "Setting velocity curve to all 1's.");
			theVeloc = new Ooscili(SR, 1.0/p[1], velarray, 2);
		}
	}

	freq = p[3];
	strikepos = p[4];
	pluck = p[5];
	maxvelocity = p[6];
	preset = int(p[7]);
	bowpress = p[8];
	modereson = p[9];
	integrate = p[10];
	pctleft = n_args > 11 ? p[11] : 0.5;                /* default is .5 */

	return nSamps();
}

int MBANDEDWG :: configure()
{
	theBar = new BandedWG(); // 50 Hz is default lowest frequency

	theBar->setFrequency(freq);
	theBar->clear(); // BGG: I took this out of the BandedWG model code

	theBar->setStrikePosition(strikepos);

	if (pluck < 1.0) theBar->setPluck(false);
	else theBar->setPluck(true);

	theBar->setBowPressure(bowpress);
	theBar->setModeResonance(modereson);
	theBar->setIntegration(integrate);

	theBar->setPreset(preset);
	theBar->noteOn(freq, maxvelocity); // this will initialize the maxVelocity

	return 0;
}

void MBANDEDWG :: doupdate()
{
	double p[13];
	update(p, 13, kAmp | kFreq | kBowPress | kModeReson | kIntegrate | kPan | kVel);

	amp = p[2] * 30.0; // for some reason this needs normalizing...
	if (amptable)
		amp *= theEnv->next(currentFrame());

	if (freq != p[3]) {
		freq = p[3];
		theBar->setFrequency(p[3]);
	}

	if (bowpress != p[8]) {
		theBar->setBowPressure(p[8]);
		bowpress = p[8];
	}

	if (modereson != p[9]) {
		theBar->setModeResonance(p[9]);
		modereson = p[9];
	}

	if (integrate != p[10]) {
		theBar->setIntegration(p[10]);
		integrate = p[10];
	}

	if (nargs > 11) pctleft = p[11];

	if (veltable) velocity = theVeloc->next(currentFrame());
	else velocity = p[12];
}

int MBANDEDWG :: run()
{
	int   i;
	float out[2];

	for (i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			doupdate();
			branch = getSkip();
		}

		out[0] = theBar->tick(velocity) * amp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pctleft);
			out[0] *= pctleft;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeMBANDEDWG()
{
	MBANDEDWG *inst;

	inst = new MBANDEDWG();
	inst->set_bus_config("MBANDEDWG");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("MBANDEDWG", makeMBANDEDWG);
}
#endif

