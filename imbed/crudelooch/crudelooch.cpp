/*  crudelooch -- very simple "interface" program for RTcmix
	uses the WAVETABLE instrument
	based on the old "looching" application from NeXT machines
	via "crudelooch.c" in the old RTcmix socket interface stuff

    demonstration of the RTcmix object -- BGG, 11/2002
*/

#define MAIN
#include <iostream.h>
#include <unistd.h>

#include <globals.h>
#include "RTcmix.h"
extern "C" {
	#include "randfuncs.h"
}

int
main(int argc, char *argv[])
{
	RTcmix *rrr;
	int i;
	double start, dur, pchval;
	double pitches[14] = {50.0, 66.667, 75.0, 100.0, 114.0, 133.333,
		150.0, 177.777, 200.0, 228.0, 266.666, 300.0, 355.555, 400.0};
	float sleepness;

	rrr = new RTcmix(44100, 2, 4096);
	rrr->printOn();
	sleep(1); // give the thread time to initialized

	// set up the instrument
	rrr->cmd("load", 1, "WAVETABLE");
	rrr->cmd("makegen", 8, 1.0, 7.0, 1000.0, 0.0, 500.0, 1.0, 500.0, 0.0);
	rrr->cmd("makegen", 10,
		2.0, 10.0, 1000.0, 1.0, 0.5, 0.25, 0.125, 0.06, 0.03, 0.015);


	tsrand(); /* seeds the random generators with time-of-day */

	while(1) {
		for (i = 0; i < 4; i++) {
			if (brrand() < 0.5) {
				start = 1.0;
				dur = brrand() * 20.0 + 5.0;
				pchval = pitches[(int)(brrand() * 14.0)];
				rrr->cmd("WAVETABLE", 5,
					start, dur, 5000.0, pchval, 0.0);

				start += brrand();
				dur += brrand();
				pchval += rrand() * (pchval * 0.0069);
				rrr->cmd("WAVETABLE", 5,
					start, dur, 5000.0, pchval, 1.0);

				start += brrand();
				dur += brrand();
				pchval += rrand() * (pchval * 0.0069);
				rrr->cmd("WAVETABLE", 5,
					start, dur, 5000.0, pchval, 0.2);

				start += brrand();
				dur += brrand();
				pchval += rrand() * (pchval * 0.0069);
				rrr->cmd("WAVETABLE", 5,
					start, dur, 5000.0, pchval, 0.8);
			}
		}
		sleepness = brrand() * 10.0 + 4.0;
		sleep((int)sleepness);
	}
}
