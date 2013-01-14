/* MCLAR - the "Clarinet" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = noise gain (0.0-1.0)
   p5 = max pressure (0.0-1.0)
   p6 = reed stiffness (0.0-1.0)
   p7 = percent of signal to left output channel [optional, default is .5]
   p8 = breath pressure table [optional]
      (must be normalized between 0.0 - 1.0)

   PField updating:
      p2 (amplitude)
         (or assumes function table 1 is breathPressure (amplitude) curve
         for the note.  If no setline or function table 1, uses flat
         amplitude curve.  A table-handle in p8 may also be used)
      p3 (frequency)
      p4 (noise amp)
      p6 (reed stiffness)
      p7 (pan)
*/

#include <Stk.h>
#include <Clarinet.h> // from the stk library

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>      /* the base class for this instrument */
#include "MCLAR.h"         /* declarations for this instrument class */
#include <rt.h>
#include <rtdefs.h>



MCLAR :: MCLAR() : Instrument()
{
	branch = 0;
	amptable = NULL;
}

MCLAR :: ~MCLAR()
{
	delete theClar;
}

int MCLAR :: init(double p[], int n_args)
{
	nargs = n_args;
	Stk::setSampleRate(SR);

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	amptable = floc(1);
	if (amptable) // the amp array has been created using makegen
		theEnv = new Ooscili(SR, 1.0/p[1], 1);

	theClar = new Clarinet(50.0); // 50 Hz is lowest freq for now
	noiseamp = p[4];
	theClar->setNoise(p[4]);
	stiff = p[6];
	theClar->setReedStiffness(p[6]);
	freq = p[3];
	theClar->noteOn(p[3], p[5]);

	pctleft = n_args > 7 ? p[7] : 0.5;                /* default is .5 */

	return nSamps();
}

void MCLAR :: doupdate()
{
	double p[9];
	update(p, 9, kAmp | kFreq | kNoise | kStiff | kPan | kBreathPress);

	// "amp" is now separate from the breath pressure.
	// breath pressure is controlled by makegen 1, or a table, or it is 1.0
	amp = p[2] * 5.0; // for some reason this needs normalizing...
	if (amptable)
		breathamp = theEnv->next(currentFrame());
	else if (nargs > 8)
		breathamp = p[8];
	else
		breathamp = 1.0;

	if (freq != p[3]) {
		theClar->setFrequency(p[3]);
		freq = p[3];
	}

	if (noiseamp != p[4]) {
		theClar->setNoise(p[4]);
		noiseamp = p[4];
	}

	if (stiff != p[6]) {
		theClar->setReedStiffness(p[6]);
		stiff = p[6];
	}

	if (nargs > 7) pctleft = p[7];
}

int MCLAR :: run()
{
	int   i;
	float out[2];

	for (i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			doupdate();
			branch = getSkip();
		}

		out[0] = theClar->tick(breathamp) * amp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pctleft);
			out[0] *= pctleft;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeMCLAR()
{
	MCLAR *inst;

	inst = new MCLAR();
	inst->set_bus_config("MCLAR");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("MCLAR", makeMCLAR);
}
#endif

