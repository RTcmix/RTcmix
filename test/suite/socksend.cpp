#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>

extern "C" {
#include <RTsockfuncs.h>
}

int theSock = 0;

void sigfunc(int sig)
{
	if (theSock != 0) {
		printf("Got signal - sending message to exit server\n");
		RTsendsock("RTcmix_off", theSock, 0);
	}
}

int
main(int argc, char *argv[])
{
	char name[100];
	int RTpid = -1;
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

	char *score;
	int arg = 1;

	if (argc <= 1) {
		fprintf(stderr, "usage: %s <-v> 'commands' <'commands'> ...\n", argv[0]);
		exit(1);
	}
	char *a = argv[arg];
	if (a[0] == '-') {
		switch (a[1]) {
		case 'v':
			verbose = 1;
			++arg;
			break;
		default:
			fprintf(stderr, "usage: %s <-v> \"score commands in quotes\"\n", argv[0]);
			exit(1);
		}
	}

	int pid = RTopensocket(0, "../../bin/CMIX");
	if (pid < 0)
		exit(1);
	sleep(1);

	sprintf(name, "localhost");

	signal(SIGINT, sigfunc);
	signal(SIGKILL, sigfunc);
	signal(SIGHUP, sigfunc);

	/* open up the socket */
	theSock = RTsock(name, 0);

    if (verbose) {
        RTsendsock("print_on", theSock, 1, 6.0);
    }
    else {
        RTsendsock("print_off", theSock, 0);
    }
	RTsendsock("rtsetparams", theSock, 3, 44100.0, 2.0, 512.0);
	for (; arg < argc; ++arg) {
		printf("sending score: '%s'\n", argv[arg]);
		RTsendsock("score", theSock, 1, argv[arg]);
	}

	printf("sleeping... ^C to stop CMIX\n");
	usleep(1000*1000*10);

	printf("stopping CMIX\n");
	RTsendsock("RTcmix_off", theSock, 0);

	return 0;
}
