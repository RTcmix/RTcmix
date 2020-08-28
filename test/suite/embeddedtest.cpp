//
//  embeddedtest.cpp
//  RTcmixTest
//
//  Created by Douglas Scott on 9/19/15.
//
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dispatch/dispatch.h>
#include <syslog.h>
#include <RTcmix_API.h>
#include <fcntl.h>

#ifndef EMBEDDED
#error This code cannot be compiled unless the system is configured for an EMBEDDED system
#endif

char message[65536];
bool done = false;

static void PrintCallback(const char *printBuffer, void *inContext)
{
	if (printBuffer != NULL) {
		strncpy(message, printBuffer, 65536);
//		syslog(LOG_ALERT, "%s\n", message);
//		fprintf(stderr, "embeddedtest: %s\n", message);
	}
}

static void DoneCallback(long long samps, void *context) {
	printf("Done - %lld samples\n", samps);
	done = true;
}

int main(int argc, char **argv)
{
	int status;
	const int rtbufsize = 512;
	const int chans = 2;
	RTcmix_setPrintCallback(PrintCallback, NULL);
	RTcmix_setFinishedCallback(DoneCallback, NULL);
	status = RTcmix_init();
	RTcmix_setPrintLevel(6);

	RTcmix_setAudioBufferFormat(AudioFormat_32BitFloat_Normalized, 2);

	status = RTcmix_setparams(44100, chans, rtbufsize, 0, 16);
	float audiobuf[rtbufsize*chans];
	__block float *audiobufptr = audiobuf;
	__block bool running = true;

	dispatch_async(dispatch_queue_create("audio queue", NULL),
				   ^{
					   while (running && RTcmix_runAudio(NULL, audiobufptr, rtbufsize) == 0) {
						   if (message[0] != 0) {
							   fprintf(stderr, "embeddedtest: %s\n", message);
							   message[0] = 0;
						   }
						   usleep(1000*5);
					   }
				   }
				   );
	
	const char *printLevelString = "print_on(6)\n";
	char scorebuf[4096];
	for (int arg = 1; arg < argc; ++arg) {
		done = false;
		int fd = open(argv[arg], O_RDONLY);
		if (fd<0) { perror(argv[arg]); exit(1); }
		long len = read(fd, scorebuf, 4096);
		close(fd);
	
		printf("sending score '%s' (length %ld bytes)\n", argv[arg], len);
		
		status = RTcmix_parseScore(scorebuf, len);
		printf("parse returned %d\n", status);
		if (status != 0) {
			printf("Offending score:\n%s\n\n", scorebuf);
            done = true;
			continue;
		}
	}

	while (!done) {
//		usleep(100*1000);
	}

	running = false;

	usleep(1000*1000*1);
	RTcmix_destroy();
	usleep(1000*1000*1);
	return status;
}
