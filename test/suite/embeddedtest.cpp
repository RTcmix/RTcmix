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

static void DoneCallback(long long samps) {
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
					   }
				   }
				   );
	
	char scorebuf[2048];
	int fd = open(argv[1], O_RDONLY);
	if (fd<0) { perror("open"); exit(1); }
	int len = read(fd, scorebuf, 2048);
	
	printf("sending score (length %d bytes)\n", len);
	status = RTcmix_parseScore(scorebuf, len);
	printf("parse returned %d\n", status);

	while (!done) {
		usleep(100*1000);
	}
	running = false;

	usleep(1000*1000*1);
	RTcmix_destroy();
	usleep(1000*1000*1);
	close(fd);
	return status;
}
