/* MBRASS - the "Brass" physical model instrument in Perry Cook/Gary Scavone's
	"stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = slide length (samps)
   p5 = lip filter (Hz)
   p6 = max pressure (0.0 - 1.0, I think...)
   p7 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.
*/

#include <Stk.h>
#include <Brass.h> // from the stk library

#include <iostream.h>        /* needed only for cout, etc. if you want it */
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>      /* the base class for this instrument */
#include "MBRASS.h"         /* declarations for this instrument class */
#include <rt.h>
#include <rtdefs.h>



MBRASS :: MBRASS() : Instrument()
{
}

MBRASS :: ~MBRASS()
{
	delete theHorn;
}

int MBRASS :: init(double p[], int n_args)
{
	Stk::setSampleRate(SR);

	nsamps = rtsetoutput(p[0], p[1], this);

	amp = p[2] * 10.0; // for some reason this needs normalizing...
	if (floc(1)) { // the amplitude array has been created in the score
		theEnv = new Ooscili(SR, 1.0/p[1], 1);
	} else {
		amparray[0] = amparray[1] = 1.0;
		advise("MBRASS", "Setting phrase curve to all 1's.");
		theEnv = new Ooscili(SR, 1.0/p[1], amparray);
	}

	theHorn = new Brass(50.0); // 50 Hz is default lowest frequency
	theHorn->setFrequency(p[3]);
	theHorn->startBlowing(p[6], 0.0);
	theHorn->setSlide((int)p[4]);
	theHorn->setLip(p[5]);

	pctleft = n_args > 7 ? p[7] : 0.5;                /* default is .5 */

	return nsamps;
}


int MBRASS :: run()
{
	int   i;
	float out[2];

	for (i = 0; i < framesToRun(); i++) {
		out[0] = theHorn->tick(theEnv->next()) * amp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pctleft);
			out[0] *= pctleft;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeMBRASS()
{
	MBRASS *inst;

	inst = new MBRASS();
	inst->set_bus_config("MBRASS");

	return inst;
}

void rtprofile()
{
	RT_INTRO("MBRASS", makeMBRASS);
}


