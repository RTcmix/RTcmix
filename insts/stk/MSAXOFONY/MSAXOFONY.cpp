/* MSAXOFONY - the "Saxofony" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = noise gain (0.0-1.0)
   p5 = max pressure (0.0-1.0)
   p6 = reed stiffness (0.0-1.0)
   p7 = reed aperture (0.0-1.0)
   p8 = blow position (0.0-1.0)
   p9 = percent of signal to left output channel [optional, default is .5]
   p10 = breath pressure table [optional]
      (must be normalized between 0.0 - 1.0)

   PField updating:
      p2 (amplitude)
         (or assumes function table 1 is breathPressure (amplitude) curve
         for the note.  If no setline or function table 1, uses flat
         amplitude curve. A table-handle in p10 may also be used)
      p3 (frequency)
      p4 (noiseamp)
      p6 (reed stiffness)
      p7 (reed aperture)
      p8 (blow position)
      p9 (pan)
      p10 (breath pressure table)
*/

#include <Stk.h>
#include <Saxofony.h> // from the stk library

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>      /* the base class for this instrument */
#include "MSAXOFONY.h"         /* declarations for this instrument class */
#include <rt.h>
#include <rtdefs.h>



MSAXOFONY :: MSAXOFONY() : Instrument()
{
	branch = 0;
	amptable = NULL;
}

MSAXOFONY :: ~MSAXOFONY()
{
	delete theSax;
}

int MSAXOFONY :: init(double p[], int n_args)
{
	nargs = n_args;
	Stk::setSampleRate(SR);

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	amptable = floc(1);
	if (amptable) // the amp array has been created using makegen
		theEnv = new Ooscili(SR, 1.0/p[1], 1);

	theSax = new Saxofony(50.0); // 50 Hz is lowest freq for now
	noiseamp = p[4];
	theSax->setNoise(p[4]);
	stiff = p[6];
	theSax->setReedStiffness(p[6]);
	aperture = p[7];
	theSax->setReedAperture(p[7]);
	blowpos = p[8];
	theSax->setBlowPosition(p[8]);
	freq = p[3];
	theSax->noteOn(p[3], p[5]);

	pctleft = n_args > 9 ? p[9] : 0.5;                /* default is .5 */

	return nSamps();
}

void MSAXOFONY :: doupdate()
{
	double p[11];
	update(p, 11, kAmp | kFreq | kNoise | kStiff | kAperture | kBlow | kPan | kBreathPress);

	// "amp" is now separate from the breath pressure.
	// breath pressure is controlled by makegen 1, or a table, or it is 1.0
	amp = p[2]*2.0; // needs normalizing
	if (amptable)
		breathamp = theEnv->next(currentFrame());
	else if (nargs > 10)
		breathamp = p[10];
	else 
		breathamp = 1.0;

	if (freq != p[3]) {
		theSax->setFrequency(p[3]);
		freq = p[3];
	}

	if (noiseamp != p[4]) {
		theSax->setNoise(p[4]);
		noiseamp = p[4];
	}

	if (stiff != p[6]) {
		theSax->setReedStiffness(p[6]);
		stiff = p[6];
	}

	if (aperture != p[7]) {
		theSax->setReedAperture(p[7]);
		aperture = p[7];
	}

	if (blowpos != p[8]) {
		theSax->setBlowPosition(p[8]);
		blowpos = p[8];
	}

	if (nargs > 9) pctleft = p[9];
}

int MSAXOFONY :: run()
{
	int   i;
	float out[2];

	for (i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			doupdate();
			branch = getSkip();
		}

		out[0] = theSax->tick(breathamp) * amp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pctleft);
			out[0] *= pctleft;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeMSAXOFONY()
{
	MSAXOFONY *inst;

	inst = new MSAXOFONY();
	inst->set_bus_config("MSAXOFONY");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("MSAXOFONY", makeMSAXOFONY);
}
#endif

