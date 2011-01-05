/*  arpeggiate -- schedule a bunch of happy STRUM notes, using the
	RTtimeit() function

    demonstration of the RTcmix object -- BGG, 11/2002
*/

#define MAIN
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <globals.h>
#include "RTcmix.h"

// declare this here so it will be global for the gonotes() function
RTcmix *rrr;

int
main(int argc, char *argv[])
{
	int i;
	void *gonotes();
	int rtsamps = 8192;
	float time = 0.8;

	for (int arg = 1; arg < argc; ++arg) {
		char *c = argv[arg];
		if (c[0] == '-') {
			if (c[1] == 'b')
				rtsamps = atoi(argv[++arg]);
			if (c[1] == 't')
				time = atof(argv[++arg]);
		}
	}

	rrr = new RTcmix(44100.0, 2, rtsamps);
//	rrr->printOn();
	rrr->printOff();
	sleep(1); // give the thread time to initialize

	// load up STRUM
	rrr->cmd("load", 1, "STRUM");

	// set up the scheduling function, update every 0.8 seconds
	RTtimeit(time, (sig_t)gonotes);

	// and don't exit!
	while(1) sleep(1);
}

void gonotes()
{
	int i;
	double pitches[14] = {6.05, 6.07, 6.10, 7.00, 7.02, 7.04,
			7.05, 7.07, 7.09, 8.00, 8.02, 8.05, 8.07, 8.10};
	double start, pch;
	double pchindex, stereo;

	start = 1.0; // give us a little lead time... not really necessary
	for (i = 0; i < 8; i++) { // schedule 8 notes 0.1 secs apart
		// it's more efficient to call a "random" directly, but hey...
		pchindex = rrr->cmd("random") * 14.0;
		pch = pitches[(int)pchindex];
		stereo = rrr->cmd("random");

		rrr->cmd("START", 9,
			start, 1.0, pch, 1.0, 0.1, 20000.0, 1.0, stereo, 1.0);
		// note the use of "deleteflag" (the last p-field) above --
		// this causes STRUM to clean up arrays, stops memory leak

		start += 0.1;
	}
}
