#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>

extern "C" {
#include "../../src/rtcmix/RTsockfuncs.h"
}

double irand(double low, double high)
{
	double frac = random() / (double) RAND_MAX;
	return low + frac * (high - low);
}

int interrupted = 0;

void sigfunc(int sig)
{
	interrupted = 1;
}

int
main(int argc, char *argv[])
{
	char name[100];
	int RTpid = -1;
	int theSock;
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

	int pid = RTopensocket(0, "CMIX");
	if (pid < 0)
		exit(1);
	sleep(2);

	sprintf(name, "localhost");

	signal(SIGINT, sigfunc);
	signal(SIGKILL, sigfunc);
	signal(SIGHUP, sigfunc);

	/* open up the socket */
	theSock = RTsock(name, 0);

	/* set up the instruments */
	RTsendsock("rtsetparams", theSock, 3, 44100.0, 2.0, 256.0);
	
	RTsendsock("load", theSock, 1, "STRUM");
	RTsendsock("load", theSock, 1, "TRANS");
	RTsendsock("load", theSock, 1, "STEREO");
	RTsendsock("rtinput", theSock, 1, "./sinetone.wav");

	RTsendsock("setline", theSock, 8, 0., 0., 1., 1., 100., 1., 110., 0.);
	RTsendsock(verbose ? "print_on" : "print_off", theSock, 0);

	checkInterval = 1000000 / maxsleep;	// about once per second
	checkCount = checkInterval;

	gettimeofday(&tv, &tz);
	base_sec = (double)tv.tv_sec;

	while(1) {
		usleep(minsleep + random() % maxsleep);	// random sleep
		num = random() % numcommands;			// random command

		switch (num) {
			case 0:
			case 3:
				pval = irand(5.0, 5.05);
				spread = irand(0.0, 1.0);
				RTsendsock("bus_config", theSock, 2, "START", "out0-1");
				RTsendsock("START", theSock, 9, 0.0, 1.0, pval, 1.0, 0.7, 5000.0, 1.0, spread, 1.0);
				totalcmds += 2;
				break;
			case 1:
				pval = irand(-0.07, 0.07);			// transp
				pval2 = irand(0.0, 1.0);			// pan
				pval3 = irand(0.05, 0.5);			// dur	
				RTsendsock("bus_config", theSock, 3, "TRANS3", "in0", "out0-1");
				RTsendsock("TRANS3", theSock, 5, 0.0, 0.0, pval3, 0.04, pval, pval2);
				totalcmds += 2;
				break;
			case 2:
				RTsendsock("bus_config", theSock, 3, "TRANS", "in0", "aox0");
				RTsendsock("bus_config", theSock, 3, "STEREO", "aix0", "out0-1");
				pval = irand(-0.04, 0.04);
				pval3 = irand(0.05, 0.5);			// dur	
				RTsendsock("TRANS", theSock, 5, 0.0, 0.0, pval3, 1.0, pval);
				pval2 = irand(0.0, 1.0);
				RTsendsock("STEREO", theSock, 5, 0.0, 0.0, pval3, 0.04, pval2);
				totalcmds += 2;
				break;
			default:
				break;
		}
		
		// Check time
		if (--checkCount == 0)
		{
		  gettimeofday(&tv, &tz);
		  sec = (double)tv.tv_sec;
		  if (interrupted || (int) (sec - base_sec) > duration)
		  {
			if (interrupted)
				printf("Shutting down...");
			else
				printf("Reached %d seconds.  Shutting down...", duration);
			fflush(stdout);
			RTsendsock("RTcmix_off", theSock, 0);
//			sleep(1);
//			RTkillsocket(theSock, RTpid);
			sleep(3); /* shut down time */
		  	printf("Exiting.\n");
			exit(0);
		  }
		  checkCount = checkInterval;
		}
	}
	return 0;
}
