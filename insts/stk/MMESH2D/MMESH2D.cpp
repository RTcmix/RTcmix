/* MMESH2D - the "Mesh2D" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = # of X points (2-12)
   p4 = # of Y points (2-12)
   p5 = xpos (0.0-1.0)
   p6 = ypos (0.0-1.0)
   p7 = decay value (0.0-1.0)
   p8 = strike energy (0.0-1.0)
   p9 = percent of signal to left output channel [optional, default is .5]

   PField updating:
      p2 (amplitude)
         (or assumes function table 1 is amplitude curve for the note.
         If no setline or function table 1, uses flat amplitude curve.)
      p9 (pan)
*/

#include <Stk.h>
#include <Mesh2D.h> // from the stk library
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include <math.h>
#include "MMESH2D.h"         /* declarations for this instrument class */
#include <rt.h>
#include <rtdefs.h>


MMESH2D :: MMESH2D()
	: branch(0), theMesh(NULL), dcblocker(NULL)
{
}

MMESH2D :: ~MMESH2D()
{
	delete theMesh;
	delete dcblocker;
}

int MMESH2D :: init(double p[], int n_args)
{
	nargs = n_args;
	Stk::setSampleRate(SR);

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	amptable = floc(1);
	if (amptable) // the amp array has been created using makegen
		theEnv = new Ooscili(SR, 1.0/p[1], 1);

	theMesh = new Mesh2D((short)p[3], (short)p[4]);
	theMesh->setInputPosition(p[5], p[6]);
	theMesh->setDecay( 0.9 + (p[7]*0.1)); // from the Mesh2D code
	theMesh->noteOn(0.0, p[8]);

	dcblocker = new Odcblock();

	pctleft = n_args > 9 ? p[9] : 0.5;                /* default is .5 */

	return nSamps();
}


int MMESH2D :: run()
{
	int   i;
	float out[2];

	for (i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			double p[10];
			update (p, 10, kAmp | kPan);

			amp = p[2];
			if (amptable)
				amp *= theEnv->next(currentFrame());

			branch = getSkip();
		}

		out[0] = dcblocker->next(theMesh->tick()) * amp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pctleft);
			out[0] *= pctleft;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeMMESH2D()
{
	MMESH2D *inst;

	inst = new MMESH2D();
	inst->set_bus_config("MMESH2D");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("MMESH2D", makeMMESH2D);
}
#endif

