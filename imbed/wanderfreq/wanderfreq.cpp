/*  wanderfreq -- start up 10 MYWAVETABLEs (modified WAVETABLE with a
	frequency-access method) and randomly choose a target freq,
	and wander towards it

    demonstration of the RTcmix object -- BGG, 11/2002

	Modified to show new Instrument functionality DS 03/2003
*/

#define MAIN
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <globals.h>
#include "RTcmix.h"
#include "MYWAVETABLE/MYWAVETABLE.h"

// declare these here so they will be global for the wander() function
RTcmix *rrr;
#define NINSTS 10
MYWAVETABLE *theWaves[NINSTS];
double curfreq[NINSTS];
double targfreq[NINSTS];
double freqinc[NINSTS];

int
main(int argc, char *argv[])
{
	int i;
	void *wander();
	double duration = 999.00;	// default to loooong time

	if (argc == 2)
		duration = atof(argv[1]);

	rrr = new RTcmix(44100.0, 2, 512);
//	rrr->printOn();
	rrr->printOff();
	sleep(1); // give the thread time to initialize

	// load up MYWAVETABLE and set the makegens
	rrr->cmd("load", 1, "./MYWAVETABLE/libMYWAVETABLE.so");
	rrr->cmd("makegen", 9,
			1.0, 24.0, 1000.0, 0.0, 0.0, 1.0, 1.0, 999.0, 1.0);
	rrr->cmd("makegen", 6, 2.0, 10.0, 1000.0, 1.0, 0.3, 0.1);

	for (i = 0; i < NINSTS; i++) {
		// isn't this cute?  we can use RTcmix score functions
		curfreq[i] = rrr->cmd("random") * 1000.0 + 100.0;
		targfreq[i] = rrr->cmd("random") * 1000.0 + 100.0;
		freqinc[i] = rrr->cmd("random") * 9.0 + 1.0;
		if (curfreq[i] > targfreq[i]) freqinc[i] = -freqinc[i];

		// start the notes, looooong duration
		theWaves[i] = (MYWAVETABLE*)rrr->cmd("MYWAVETABLE", 5,
			0.0, duration, 30000.0/(double)NINSTS, curfreq[i],
			(double)i*(1.0/(double)NINSTS));
		theWaves[i]->ref();	// Keep these from being destroyed
	}

/* ok, I know scheduling a function to fire every 0.02 seconds is a little
   crazed, but it's a *DEMO*, laddie!  -- best place to do this would be
   in the instrument itself and just set targets */
	// set up the scheduling function, update every 0.02 seconds
	RTtimeit(0.02, (sig_t)wander);

	// and don't exit!
	while(1) sleep(1);
}

void wander()
{
	bool wavesLeft = false;
	for (int i = 0; i < NINSTS; i++) {
		if (theWaves[i] == NULL)
			continue;		// Already released this one
	    else if (theWaves[i]->isDone())
		{
			theWaves[i]->unref();	// Release our hold on this
			theWaves[i] = NULL;
			continue;
		}

		wavesLeft = true;
		curfreq[i] += freqinc[i];
		// this is where we change the frequency of each executing note
		theWaves[i]->setfreq(curfreq[i]);

		if (freqinc[i] < 0.0) {
			if (curfreq[i] < targfreq[i]) {
				targfreq[i] = rrr->cmd("random")*1000.0 + 100.0;
				freqinc[i] = rrr->cmd("random")*9.0 + 1.0;
				if (curfreq[i] > targfreq[i])
					freqinc[i] = -freqinc[i];
			}
		} else {
			if (curfreq[i] > targfreq[i]) {
				targfreq[i] = rrr->cmd("random")*1000.0 + 100.0;
				freqinc[i] = rrr->cmd("random")*9.0 + 1.0;
				if (curfreq[i] > targfreq[i])
					freqinc[i] = -freqinc[i];
			}
		}
	}
	if (!wavesLeft)
	{
		printf("All waves have finished.  Exiting.\n");
		rrr->close();
		exit(0);
	}
}
