/*  mixit -- randomly mix together short fragments of a soundfile

    demonstration of the RTcmix object -- BGG, 11/2002
*/

#define MAIN

#include <iostream>
#include <unistd.h>

#include <globals.h>
#include "RTcmix.h"
extern "C" {
	#include "randfuncs.h"
}

using namespace std;

int
main(int argc, char *argv[])
{
	RTcmix *rrr;
	char name[100];
	int i;
	double filedur, dur, insk, outsk;

	cout << "Enter the name of the soundfile to scramble: ";
	cin >> name;

	rrr = new RTcmix();
	rrr->printOff();
	sleep(1); // give the thread time to initialized

	rrr->cmd("load", 1, "STEREO");
	rrr->cmd("rtinput", 1, name);
	filedur = rrr->cmd("DUR");
	rrr->cmd("setline", 6, 0.0, 0.0, 0.05, 1.0, 0.1, 0.0);
	rrr->cmd("reset", 1, 44100.0);

	tsrand(); // seed wthe random number generators with time-of-day

	outsk = 0.0;
	dur = 0.01;
	while(1) {
		outsk = 0.0;
		for (i = 0; i < 100; i++) {
			insk = brrand() * filedur;
			rrr->cmd("STEREO", 6, outsk, insk, dur, 1.0, 0.0, 1.0);
			outsk += 0.01;
		}
		sleep(1);
	}
}
