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

   Assumes function table 1 is breathPressure (amplitude) curve for the note.
   Or you can just call setline. If no setline or function table 1, uses
   flat curve.
*/

#include <Stk.h>
#include <Clarinet.h> // from the stk library

#include <iostream.h>        /* needed only for cout, etc. if you want it */
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
}

MCLAR :: ~MCLAR()
{
	delete theClar;
}

int MCLAR :: init(double p[], int n_args)
{
	Stk::setSampleRate(SR);

	nsamps = rtsetoutput(p[0], p[1], this);

	amp = p[2] * 5.0; // for some reason this needs normalizing...
	if (floc(1)) { // the amplitude array has been created in the score
		theEnv = new Ooscili(SR, 1.0/p[1], 1);
	} else {
		amparray[0] = amparray[1] = 1.0;
		advise("MCLAR", "Setting phrase curve to all 1's.");
		theEnv = new Ooscili(SR, 1.0/p[1], amparray);
	}

	theClar = new Clarinet(50.0); // 50 Hz is lowest freq for now
	theClar->setNoise(p[4]);
	theClar->setReedStiffness(p[6]);
	theClar->noteOn(p[3], p[5]);

	pctleft = n_args > 7 ? p[7] : 0.5;                /* default is .5 */

	return nsamps;
}


int MCLAR :: run()
{
	int   i;
	float out[2];

	for (i = 0; i < framesToRun(); i++) {
		out[0] = theClar->tick(theEnv->next()) * amp;

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

void rtprofile()
{
	RT_INTRO("MCLAR", makeMCLAR);
}


