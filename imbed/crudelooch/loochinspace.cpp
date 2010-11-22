/*  loochinspace -- modified version of crudelooch that
    moves WAVETABLE sounds around in space.
    DS, 07/2005
	Optional arguments: -s <seed> to see the random number generator.
*/

#define MAIN
#include <unistd.h>
#include <stdlib.h>

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
	bool verbose = false;

	rrr = new RTcmix(44100, 2, 2048);
	sleep(1); // give the thread time to initialized

	if (argc >= 3 && argv[1][0] == '-') {
		if (argv[1][1] == 'v') {
			verbose = true;
		}
		else if (argv[1][1] == 's')
		{
			int seed = atoi(argv[2]);
			srrand(seed);
		}
		else
			tsrand(); /* seeds the random generators with time-of-day */
	}
	else
		tsrand(); /* seeds the random generators with time-of-day */

	if (verbose)
		rrr->printOn();
	else
		rrr->printOff();

	// set up the instrument
	rrr->cmd("load", 1, "WAVETABLE");
	rrr->cmd("makegen", 8, 1.0, 7.0, 1000.0, 0.0, 500.0, 1.0, 500.0, 0.0);
	rrr->cmd("makegen", 10,
		2.0, 10.0, 1000.0, 1.0, 0.5, 0.25, 0.125, 0.06, 0.03, 0.015);

	// set up room
	rrr->cmd("load", 1, "MMOVE");
    double fr=120, rt=120, rear=-120, lft=-120, top=100, abs=5, rvbtime=5;
	double gain = 80;
    rrr->cmd("space", 7, fr, rt, rear, lft, top, abs, rvbtime);
	rrr->cmd("mikes", 2, 45, 0.5);
	rrr->cmd("set_attenuation_params", 3, 1.0, 100.0, 1.4);
	rrr->cmd("threshold", 1, 0.05);
	
	// RVB always takes aux 9-10 as input.
	rrr->cmd("bus_config", 3, "RVB", "aix9-10", "out0-1");

	// Start reverb
	rrr->cmd("RVB", 4, 0.0, 0.0, 10000000.0, 1.0);
	
	typedef struct _Buses { const char *wave; const char *mmovein; } Buses;
	static const Buses buses[] = {
		"aox0", "aix0",
		"aox1", "aix1",
		"aox2", "aix2",
		"aox3", "aix3",
		"aox4", "aix4",
		"aox5", "aix5",
		"aox6", "aix6",
		"aox7", "aix7"
	};
	int noteIndex = 0;
	
	while(1) {
		for (i = 0; i < 4; i++) {
			if (brrand() < 0.5) {
			    double startAngle, endAngle;
				double startDist = 70.0, endDist = 70.0;
				start = 1.0;
				dur = brrand() * 20.0 + 5.0;
				pchval = pitches[(int)(brrand() * 14.0)];
				int busIndex = noteIndex++ % 8;
				rrr->cmd("bus_config", 2, "WAVETABLE", buses[busIndex].wave);
				// MMOVE always puts output into aux 9-10
				rrr->cmd("bus_config", 3, "MMOVE", buses[busIndex].mmovein, "aox9-10");

			    for (int events = 0; events < 4; ++events) {
					rrr->cmd("WAVETABLE", 5,
						0.0, dur, 5000.0, pchval, 0.0);
					start += brrand();
					dur += brrand();
					pchval += rrand() * (pchval * 0.0069);				
				}
				startAngle = brrand() * 180.0;
				endAngle = brrand() * 180.0;
				rrr->cmd("path", 6, 0.0, startDist, startAngle, 1.0, endDist, endAngle);
				rrr->cmd("MMOVE", 6, start, 0.0, dur+start, gain, 2.0, 0.0);
			}
		}
		sleepness = brrand() * 10.0 + 4.0;
		sleep((int)sleepness);
	}
}
