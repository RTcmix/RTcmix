/* MBOWED - the "Bowed" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = vibrato freq low (Hz)
   p5 = vibrato freq high (Hz)
   p6 = vibrato depth (% of base frequency [decimal notation 0.1 == 10%])
   p7 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is breathPressure (amplitude) curve for the note.
   Or you can just call setline. If no setline or function table 1, uses
   flat curve.

   function table 2 = bow pressure tracking curve (0.0-1.0)
   function table 3 = bow position tracking curve (0.0-1.0)
   function table 4 = vibrato waveform
*/

#include <Stk.h>
#include <Bowed.h> // from the stk library

#include <iostream.h>        /* needed only for cout, etc. if you want it */
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>      /* the base class for this instrument */
#include "MBOWED.h"         /* declarations for this instrument class */
#include <rt.h>
#include <rtdefs.h>



MBOWED :: MBOWED() : Instrument()
{
}

MBOWED :: ~MBOWED()
{
	delete theBow;
}

int MBOWED :: init(double p[], int n_args)
{
	Stk::setSampleRate(SR);

	nsamps = rtsetoutput(p[0], p[1], this);

	amp = p[2] * 5; // for some reason this needs normalizing...
	if (floc(1)) { // the amplitude array has been created in the score
		theEnv = new Ooscili(SR, 1.0/p[1], 1);
	} else {
		amparray[0] = amparray[1] = 1.0;
		advise("MBOWED", "Setting phrase curve to all 1's.");
		theEnv = new Ooscili(SR, 1.0/p[1], amparray);
	}

	theRand = new Orand();

	thePressure = new Ooscili(SR, 1.0/p[1], 2);
	thePosition = new Ooscili(SR, 1.0/p[1], 3);

	viblo = p[4];
	vibhi = p[5];
	theVib = new Ooscili(SR, theRand->range(viblo, vibhi), 4);
	vibupdate = 0;

	freqbase = p[3] - (p[6] * p[3]);
	freqamp = 2.0 * (p[6] * p[3]);

	theBow = new Bowed(50.0); // 50 Hz is lowest freq for now

	theBow->noteOn(p[3], p[5]);

	pctleft = n_args > 7 ? p[7] : 0.5;                /* default is .5 */

	return nsamps;
}


int MBOWED :: run()
{
	int   i;
	float out[2];

	for (i = 0; i < framesToRun(); i++) {
		if (--vibupdate < 0) {
			float vibfreq = theRand->range(viblo, vibhi);
			theVib->setfreq(vibfreq);
			vibupdate = (int)(SR/vibfreq);
		}

		out[0] = theBow->tick(theEnv->next()) * amp;
		theBow->setFrequency(freqbase + (freqamp * ( (theVib->next()+2.0) * 0.5 )));
		theBow->setBowPressure(thePressure->next(currentFrame()));
		theBow->setBowPosition(thePosition->next(currentFrame()));

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pctleft);
			out[0] *= pctleft;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeMBOWED()
{
	MBOWED *inst;

	inst = new MBOWED();
	inst->set_bus_config("MBOWED");

	return inst;
}

void rtprofile()
{
	RT_INTRO("MBOWED", makeMBOWED);
}


