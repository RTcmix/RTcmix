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

*/

#include <Stk.h>
#include <Shakers.h> // from the stk library

#include <iostream.h>        /* needed only for cout, etc. if you want it */
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
}

MSHAKERS :: ~MSHAKERS()
{
	delete theShake;
}

int MSHAKERS :: init(double p[], int n_args)
{
	Stk::setSampleRate(SR);

	nsamps = rtsetoutput(p[0], p[1], this);

	amp = p[2] * 5.0; // for some reason this needs normalizing...

	theShake = new Shakers();
	theShake->setShakerType((int)p[7]);
	theShake->setEnergy(p[3]);
	theShake->setDecay(p[4]);
	theShake->setNumObjects(p[5]);
	theShake->setResonance(p[6]);

	pctleft = n_args > 8 ? p[8] : 0.5;                /* default is .5 */

	return nsamps;
}


int MSHAKERS :: run()
{
	int   i;
	float out[2];

	for (i = 0; i < framesToRun(); i++) {
		out[0] = theShake->tick() * amp;

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

void rtprofile()
{
	RT_INTRO("MSHAKERS", makeMSHAKERS);
}


