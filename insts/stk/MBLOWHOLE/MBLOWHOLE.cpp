/* MBLOWHOLE - the "BlowHole" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = noise gain (0.0-1.0)
   p5 = max pressure (0.0-1.0)
   p6 = reed stiffness (0.0-1.0)
   p7 = Tonehole state (1 == "open"; 0 == "closed")
   p8 = Register vent state (1 == "open"; 0 == "closed")
   p9 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is breathPressure (amplitude) curve for the note.
   Or you can just call setline. If no setline or function table 1, uses
   flat curve.
*/

#include <Stk.h>
#include <BlowHole.h> // from the stk library

#include <iostream.h>        /* needed only for cout, etc. if you want it */
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>      /* the base class for this instrument */
#include "MBLOWHOLE.h"         /* declarations for this instrument class */
#include <rt.h>
#include <rtdefs.h>



MBLOWHOLE :: MBLOWHOLE() : Instrument()
{
}

MBLOWHOLE :: ~MBLOWHOLE()
{
	delete theClar;
}

int MBLOWHOLE :: init(double p[], int n_args)
{
	Stk::setSampleRate(SR);

	nsamps = rtsetoutput(p[0], p[1], this);

	amp = p[2] * 3.0; // for some reason this needs normalizing...
	if (floc(1)) { // the amplitude array has been created in the score
		theEnv = new Ooscili(SR, 1.0/p[1], 1);
	} else {
		amparray[0] = amparray[1] = 1.0;
		advise("MBLOWHOLE", "Setting phrase curve to all 1's.");
		theEnv = new Ooscili(SR, 1.0/p[1], amparray);
	}

	theClar = new BlowHole(50.0); // 50 Hz is lowest freq for now
	theClar->setNoise(p[4]);
	theClar->setReedStiffness(p[6]);
	theClar->setTonehole(p[7]);
	theClar->setVent(p[8]);
	theClar->noteOn(p[3], p[5]);

	pctleft = n_args > 9 ? p[9] : 0.5;                /* default is .5 */

	return nsamps;
}


int MBLOWHOLE :: run()
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


Instrument *makeMBLOWHOLE()
{
	MBLOWHOLE *inst;

	inst = new MBLOWHOLE();
	inst->set_bus_config("MBLOWHOLE");

	return inst;
}

void rtprofile()
{
	RT_INTRO("MBLOWHOLE", makeMBLOWHOLE);
}


