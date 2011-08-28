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
#include <RTcmix.h>
#include "MYWAVETABLE/MYWAVETABLE.h"
#include <PField.h>
#include <PFieldSet.h>

// declare these here so they will be global for the wander() function
RTcmix *rrr;
#define NINSTS 10
MYWAVETABLE *theWaves[NINSTS];
RTNumberPField *ampFields[NINSTS];
RTNumberPField *pitchFields[NINSTS];
RTNumberPField *spreadFields[NINSTS];
double targamp[NINSTS];
double ampinc[NINSTS];
double targfreq[NINSTS];
double freqinc[NINSTS];

const double maxamp = 60000.0/(double)NINSTS;

int
main(int argc, char *argv[])
{
	int i;
	void *wander();
	double duration = 999.00;	// default to loooong time

	if (argc == 2)
		duration = atof(argv[1]);

	rrr = new RTcmix(44100.0, 2, 256);
//	rrr->printOn();
//	rrr->printOff();
	sleep(1); // give the thread time to initialize

	// load up MYWAVETABLE and set the makegens
	rrr->cmd("load", 1, "./MYWAVETABLE/libMYWAVETABLE.so");
	rrr->cmd("makegen", 9,
			1.0, 24.0, 1000.0, 0.0, 0.0, 1.0, 1.0, 999.0, 1.0);
	rrr->cmd("makegen", 6, 2.0, 10.0, 1000.0, 1.0, 0.3, 0.1);
	rrr->cmd("reset", 1, 100);

	ConstPField *startField = new ConstPField(0);
	ConstPField *durField = new ConstPField(duration);

	for (i = 0; i < NINSTS; i++) {
		// isn't this cute?  we can use RTcmix score functions
		double curamp = rrr->cmd("random") * maxamp;
		targamp[i] = rrr->cmd("random") * maxamp;
		ampinc[i] = rrr->cmd("random") * 0.1;
		if (curamp > targamp[i]) ampinc[i] = -ampinc[i];

		double curfreq = rrr->cmd("random") * 1000.0 + 100.0;
		targfreq[i] = rrr->cmd("random") * 1000.0 + 100.0;
		freqinc[i] = rrr->cmd("random") * 9.0 + 1.0;
		if (curfreq > targfreq[i]) freqinc[i] = -freqinc[i];

		// Here is my first attempt at the new imbedded PField system.
		// First we declare a PFieldSet.  We will need one per instrument
		// because we pass unique PField objects to each instrument.
		PFieldSet waveSet(5);
		// We load the same start and dur pfields into all sets, because they
		// are constants.  PFields are reference counted.
		waveSet.load(startField, 0);
		waveSet.load(durField, 1);
		// Now we create our 3 variable PField instances, ref them for ourselves,
		// and load them into the set.
		ampFields[i] = new RTNumberPField(curamp);
		ampFields[i]->ref();
		waveSet.load(ampFields[i], 2);

		pitchFields[i] = new RTNumberPField(curfreq);
		pitchFields[i]->ref();
		waveSet.load(pitchFields[i], 3);

		spreadFields[i] = new RTNumberPField((double)i*(1.0/(double)NINSTS));
		spreadFields[i]->ref();
		waveSet.load(spreadFields[i], 4);

		// start the notes, looooong duration
		theWaves[i] = (MYWAVETABLE*)rrr->cmd("MYWAVETABLE", waveSet);
		theWaves[i]->ref();	// Keep these from being destroyed.  We only use these
							// to determine whether they are still playing.
	}

	// set up the scheduling function, update every 0.04 seconds
	RTtimeit(0.04, (sig_t)wander);

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
		{	// Release our hold on these
			ampFields[i]->unref();
			pitchFields[i]->unref();
			spreadFields[i]->unref();
			theWaves[i]->unref();
			theWaves[i] = NULL;
			continue;
		}

		wavesLeft = true;
		
		// this is where we change the amp & frequency of each executing note
		
		double curamp = ampFields[i]->offset(ampinc[i]);

		if (ampinc[i] < 0.0) {
			if (curamp < targamp[i]) {
				targamp[i] = rrr->cmd("random")*maxamp;
				ampinc[i] = rrr->cmd("random")*.1;
				if (curamp > targamp[i])
					ampinc[i] = -ampinc[i];
			}
		} else {
			if (curamp > targamp[i]) {
				targamp[i] = rrr->cmd("random")*maxamp;
				ampinc[i] = rrr->cmd("random")*.1;
				if (curamp > targamp[i])
					ampinc[i] = -ampinc[i];
			}
		}

//		printf("offsetting inst %d (PF %p) by %g ", i, pitchFields[i], freqinc[i]);
		double curfreq = pitchFields[i]->offset(freqinc[i]);
//		printf("to %g\n", curfreq);

		if (freqinc[i] < 0.0) {
			if (curfreq < targfreq[i]) {
				targfreq[i] = rrr->cmd("random")*1000.0 + 100.0;
				freqinc[i] = rrr->cmd("random")*9.0 + 1.0;
				if (curfreq > targfreq[i])
					freqinc[i] = -freqinc[i];
			}
		} else {
			if (curfreq > targfreq[i]) {
				targfreq[i] = rrr->cmd("random")*1000.0 + 100.0;
				freqinc[i] = rrr->cmd("random")*9.0 + 1.0;
				if (curfreq > targfreq[i])
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
