#define MAIN
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include <globals.h>
#include "RTcmix.h"

double irand(double low, double high)
{
    double frac = random() / (double) RAND_MAX;
	return low + frac * (high - low);
}

int
main(int argc, char *argv[])
{
	RTcmix *rrr;
	int term;
	int num;
	unsigned totalcmds = 0;
	int numcommands = 4;
	int minsleep = 1000;	// in microseconds
	int maxsleep = 10000;
	int checkCount, checkInterval;
	int i;
	int verbose = 0;
	int duration = 10000;	// in seconds
	double start, pval, pval2, pval3, spread;
	struct timeval tv;
	struct timezone tz;
	double base_sec, sec, usec;

	for (int arg = 1; arg < argc; ++arg)
	{
		char *a = argv[arg];
		if (a[0] == '-') {
			switch (a[1]) {
			case 's':
				srand(atoi(argv[++arg]));
				break;
			case 'v':
				verbose = 1;
				break;
			case 'd':
				duration = atoi(argv[++arg]);
				printf("Running for %d seconds...\n", duration);
				break;
			default:
				fprintf(stderr, "usage: %s <-s randseed> <-v>\n", argv[0]);
				exit(1);
			}
		}
		else {
			fprintf(stderr, "usage: %s <-s randseed> <-v>\n", argv[0]);
			exit(1);
		}
	}

	rrr = new RTcmix(44100.0, 2, 256);
	if (verbose)
		rrr->printOn();
	else
		rrr->printOff();
	sleep(1); // give the thread time to initialized

	rrr->cmd("load", 1, "STRUM");
	rrr->cmd("load", 1, "TRANS");
	rrr->cmd("load", 1, "STEREO");
	rrr->cmd("rtinput", 1, "./sinetone.wav");

	rrr->cmd("setline", 8, 0., 0., 1., 1., 100., 1., 110., 0.);
	
	checkInterval = 1000000 / maxsleep;	// about once per second
	checkCount = checkInterval;

	gettimeofday(&tv, &tz);
	base_sec = (double)tv.tv_sec;
//	usec = (double)tv.tv_usec;

	while(1) {
		usleep(minsleep + random() % maxsleep);	// random sleep
		num = random() % numcommands;			// random command

		switch (num) {
			case 0:
			case 3:
				pval = irand(5.0, 5.05);
				spread = irand(0.0, 1.0);
				rrr->cmd("bus_config", 2, "START", "out0-1");
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 5000.0, 1.0, spread);
				totalcmds += 2;
				break;
			case 1:
				pval = irand(-0.07, 0.07);			// transp
				pval2 = irand(0.0, 1.0);			// pan
				pval3 = irand(0.05, 0.5);			// dur	
				rrr->cmd("bus_config", 3, "TRANS3", "in0", "out0-1");
				rrr->cmd("TRANS3", 5, 0.0, 0.0, pval3, 0.04, pval, pval2);
				totalcmds += 2;
				break;
			case 2:
				rrr->cmd("bus_config", 3, "TRANS", "in0", "aox0");
				rrr->cmd("bus_config", 3, "STEREO", "aix0", "out0-1");
				pval = irand(-0.04, 0.04);
				pval3 = irand(0.05, 0.5);			// dur	
				rrr->cmd("TRANS", 5, 0.0, 0.0, pval3, 1.0, pval);
				pval2 = irand(0.0, 1.0);
				rrr->cmd("STEREO", 5, 0.0, 0.0, pval3, 0.04, pval2);
				totalcmds += 2;
				break;
			case 27: /* ESC shuts it down */
				return(1);
			default:
				break;
		}
		// Check time
		if (--checkCount == 0)
		{
		  gettimeofday(&tv, &tz);
		  sec = (double)tv.tv_sec;
		  if ((int) (sec - base_sec) > duration)
		  {
		  	printf("Reached %d seconds.  Exiting.\n", duration);
			exit(0);
		  }
		  checkCount = checkInterval;
		}
	}
}
