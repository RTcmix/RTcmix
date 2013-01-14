/* MSHAKERS - the "Shakers" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = energy (0.0-1.0)
   p4 = decay (0.0-1.0)
   p5 = # of objects (0.0-1.0)
   p6 = resonance freq (0.0-1.0)
   p7 = instrument selection (0-22 -- see "instruments" file for listing)
   p8 = percent of signal to left output channel [optional, default is .5]

   PField updating:
      p2 (amplitude)
      p3 (energy) -- changing this keeps the instrument shaking
      p4 (decay) -- if this is > 1.0, keeps it going (be careful!)
         p4 interacts with the natural decay of the shaker
      p5 (# of objects)
      p6 (resonance frequency)
      p8 (pan)
*/

#include <Stk.h>
#include <Shakers.h> // from the stk library

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>      /* the base class for this instrument */
#include "MSHAKERS.h"         /* declarations for this instrument class */
#include <rt.h>
#include <rtdefs.h>



MSHAKERS :: MSHAKERS() : Instrument()
{
	branch = 0;
}

MSHAKERS :: ~MSHAKERS()
{
	delete theShake;
}

int MSHAKERS :: init(double p[], int n_args)
{
	nargs = n_args;
	Stk::setSampleRate(SR);

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	amp = p[2]; // for update checking
	aamp = p[2] * 5.0; // for some reason this needs normalizing...

	theShake = new Shakers();
	theShake->setShakerType((int)p[7]);
	energy = p[3];
	theShake->setEnergy(p[3]);
	decay = p[4];
	theShake->setDecay(p[4]);
	nobjects = p[5];
	theShake->setNumObjects(p[5]);
	resfreq = p[6];
	theShake->setResonance(p[6]);

	pctleft = n_args > 8 ? p[8] : 0.5;                /* default is .5 */

	return nSamps();
}

void MSHAKERS :: doupdate()
{
	double p[9];
	update(p, 9, kAmp | kEnergy | kDecay | kNobjs | kRfreq | kPan);

	if (amp != p[2]) {
		aamp = p[2] * 5.0;
		amp = p[2];
	}

	if (energy != p[3]) {
		theShake->setEnergy(p[3]);
		energy = p[3];
	}

	if (decay != p[4]) {
		theShake->setDecay(p[4]);
		decay = p[4];
	}

	if (nobjects != p[5]) {
		theShake->setNumObjects(p[5]);
		nobjects = p[5];
	}

	if (resfreq != p[6]) {
		theShake->setResonance(p[6]);
		resfreq = p[6];
	}

	if (nargs > 8) pctleft = p[8];
}

int MSHAKERS :: run()
{
	int   i;
	float out[2];

	for (i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			doupdate();
			branch = getSkip();
		}

		out[0] = theShake->tick() * aamp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pctleft);
			out[0] *= pctleft;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeMSHAKERS()
{
	MSHAKERS *inst;

	inst = new MSHAKERS();
	inst->set_bus_config("MSHAKERS");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("MSHAKERS", makeMSHAKERS);
}
#endif

