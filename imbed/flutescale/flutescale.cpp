/*  flutescale -- schedule and play a simple scale using the METAFLUTE
	instrument (one of Perry Cook's physical models)

    demonstration of the RTcmix object -- BGG, 11/2002
*/

#define MAIN
#include <unistd.h>

#include <globals.h>
#include "RTcmix.h"

int
main(int argc, char *argv[])
{
	RTcmix *rrr;

	rrr = new RTcmix();
	rrr->printOn();
	sleep(1); // give the thread time to initialized

	rrr->cmd("load", 1, "METAFLUTE");
	rrr->cmd("makegen", 7, 1.0, 24.0, 1000.0, 0.0, 1.0, 1.0, 1.0);
	rrr->cmd("makegen", 11, 2.0, 24.0, 1000.0,
		0.0, 0.0, 0.05, 1.0, 0.95, 1.0, 1.0, 0.0);

	rrr->cmd("SFLUTE", 7, 0.0, 1.0, 0.1, 106.0, 25.0, 5000.0, 0.5);
	rrr->cmd("SFLUTE", 7, 1.0, 1.0, 0.1, 95.0, 21.0, 5000.0, 0.5);
	rrr->cmd("SFLUTE", 7, 2.0, 1.0, 0.1, 89.0, 19.0, 5000.0, 0.5);
	rrr->cmd("SFLUTE", 7, 3.0, 1.0, 0.1, 75.0, 19.0, 5000.0, 0.5);
	rrr->cmd("SFLUTE", 7, 4.0, 1.0, 0.1, 70.0, 15.0, 5000.0, 0.5);
	rrr->cmd("SFLUTE", 7, 5.0, 1.0, 0.1, 67.0, 16.0, 5000.0, 0.5);
	rrr->cmd("SFLUTE", 7, 6.0, 1.0, 0.1, 56.0, 17.0, 5000.0, 0.5);
	rrr->cmd("SFLUTE", 7, 7.0, 1.0, 0.1, 53.0, 25.0, 5000.0, 0.5);

	sleep(8);
}
