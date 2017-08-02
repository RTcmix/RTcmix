/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
/* Command line score player that lets you trigger up to 9 different scores
	from the computer keyboard. They can play simultaneously.
	This is to test certain conditions that happen when RTcmix receives multiple
	scores during a run, as if they are part of one large score. There are likely
	to be cases where the scores interfere with each other.

	The screen handling code comes from Brad's old happykeys program.

	John Gibson, 8/2/17
*/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <curses.h>
#include <fcntl.h>
#include <portaudio.h>

#define EMBEDDEDAUDIO
#include <RTcmix_API.h>

#define MAX_SCORES 9
#define SAMPLE_RATE 44100.0
#define FRAMES_PER_BUF 512
#define MIN_FRAMES_PER_BUF 64	// I think this may be right for PortAudio
#define INPUT_CHANS 0
#define OUTPUT_CHANS 2
#define BUS_COUNT 32

PaStream *stream = NULL;
int verbose = 0;
float rescaleFactor = 1.0;
float sampleRate = SAMPLE_RATE;
int framesPerBuffer = FRAMES_PER_BUF;
int numOutChannels = OUTPUT_CHANS;
int numInternalBuses = BUS_COUNT;
char *progname = NULL;
const char *scoreNames[MAX_SCORES];
char *scores[MAX_SCORES];
int numScores = 0;

#define USAGE_MSG                                                   " \n\
usage: %s [options] score1 [score2 ... scoreN]                        \n\
                                                                      \n\
  option   description                               default          \n\
  --------------------------------------------------------------------\n\
  -a NUM   amplitude multiplier                      1.0              \n\
  -b NUM   number of RTcmix internal buses           32               \n\
  -B NUM   frames per buffer                         512              \n\
  -c NUM   number of output channels                 2                \n\
  -h       show this help text                                        \n\
  -r NUM   sampling rate                             44100.0          \n\
  -v       print settings before playing                              \n\
                                                                      \n\
  Trigger scores using keys for numbers 1-9.                          \n\
                                                                      \n\
  You must leave a space between a - flag and its arg. -d3 won't work.\n\
  This program won't work for any scores that take live audio input or\n\
  invoke MIDI, OSC, or the mouse or display apps. There is no checking\n\
  for these dependencies.                                             \n\
  The amplitude multiplier is applied after RTcmix generates samples, \n\
  so it does not prevent clipping.                                    \n\
                                                                      \n\
"

/* this function called by high-priority system audio thread
	(CoreAudio on macOS)
*/
static int paCallback(const void *input,
							void *output,
							unsigned long frames,
							const PaStreamCallbackTimeInfo *timeInfo,
							PaStreamCallbackFlags statusFlags,
							void *userData)
{
	RTcmix_runAudio((void *)input, output, frames);
	if (rescaleFactor != 1.0) {
		float *buf = (float *) output;
		unsigned long len = frames * numOutChannels;
		for (unsigned long i = 0; i < len; i++)
			*buf++ *= rescaleFactor;
	}
	return paContinue;
}

int initAudio()
{
	PaError err = Pa_Initialize();
	if (err != paNoError)
		goto error;

	err = Pa_OpenDefaultStream(&stream,
										INPUT_CHANS,
										numOutChannels,
										paFloat32,
										sampleRate,
										framesPerBuffer,
										paCallback,
										NULL);
	if (err != paNoError)
		goto error;

	err = Pa_StartStream(stream);
	if (err != paNoError)
		goto error;

	return 0;
error:
	fprintf(stderr, "PortAudio init failed (%s)\n", Pa_GetErrorText(err));
	return -1;
}

int initRTcmix()
{
	char str[64];

	int result = RTcmix_init();
	if (result)
		goto error;

	result = RTcmix_setAudioBufferFormat(AudioFormat_32BitFloat_Normalized,
					numOutChannels);
	if (result)
		goto error;

	result = RTcmix_setparams(sampleRate,
									  numOutChannels,
									  framesPerBuffer,
									  0,	// taking input?
									  numInternalBuses);
	if (result)
		goto error;

	// Clipping reports mess up our curses screen
	strncpy(str, "set_option(\"report_clipping = false\");\n", 64);
	str[63] = 0;
	RTcmix_parseScore(str, strlen(str));

	return 0;
error:
	fprintf(stderr, "RTcmix init failed\n");
	return -1;
}

int deleteAudio()
{
	PaError err = Pa_StopStream(stream);
	if (err != paNoError)
		goto error;

	err = Pa_CloseStream(stream);
	if (err != paNoError)
		goto error;

	err = Pa_Terminate();
	if (err != paNoError)
		goto error;

	return 0;
error:
	fprintf(stderr, "PortAudio delete failed (%s)\n", Pa_GetErrorText(err));
	return -1;
}

int deleteRTcmix()
{
	return RTcmix_destroy();
}

int playScores()
{
	int i;
	char str[256];

	// open terminal
	int term = open("/dev/tty", O_RDWR);
	if (term < 0) {
		fprintf(stderr, "Error configuring keyboard (%s)\n", strerror(errno));
		return -1;
	}

	// set up terminal for raw character capturing
	initscr();
	cbreak();

	// print instructions
	if (numScores == 1)
		addstr("Type number 1 to play the score, ESC to quit.\n");
	else {
		snprintf(str, 256, "Type numbers 1-%d to play the scores, ESC to quit.\n",					numScores);
		addstr(str);
	}
	for (i = 0; i < numScores; i++) {
		snprintf(str, 256, "[%d] %s\n", i + 1, scoreNames[i]);
		addstr(str);
	}
	addstr("\n");

	while (1) {
		int c = getch();
		if (c >= 49 && c <= 57) {
			int scoreNum = c - 48;
			if (scoreNum <= numScores) {
				i = scoreNum - 1;	// zero-based index
				RTcmix_parseScore(scores[i], strlen(scores[i]));
			}
		}
		else if (c == 27) {
			printf("\n\nquitting...\n");
			fflush(stdout);
			endwin();
			break;
		}
	}
	return 0;
}

int loadScores()
{
	int i;
	for (i = 0; i < numScores; i++) {
		int result;
		long len;
		size_t bytes;
		char *buf;

		const char *filename = scoreNames[i];
		result = access(filename, F_OK | R_OK);
		if (result != 0) {
			fprintf(stderr, "Error accessing `%s' (%s)\n", filename,
						strerror(errno));
			return -1;
		}
		FILE *file = fopen(filename, "r");
		if (!file) {
			fprintf(stderr, "Error opening `%s'\n", filename);
			return -1;
		}

		result = fseek(file, 0, SEEK_END);
		if (result != 0) {
			fprintf(stderr, "Error seeking end of `%s' (%s)\n", filename,
						strerror(errno));
			return -1;
		}
		len = ftell(file);
		if (len == -1) {
			fprintf(stderr, "Error finding length of `%s' (%s)\n", filename,
						strerror(errno));
			return -1;
		}
		rewind(file);

		buf = (char *) malloc(sizeof(char) * (len + 1));
		if (buf == NULL) {
			fprintf(stderr, "Error allocating memory for `%s' (%s)\n",
						filename, strerror(errno));
			return -1;
		}
		bytes = fread(buf, sizeof(char), len, file);
		if (bytes != len) {
			fprintf(stderr, "Error reading `%s'\n", filename);
			free(buf);
			return -1;
		}
		buf[len] = 0;
		fclose(file);
		scores[i] = buf;
	}
	return 0;
}

void printSettings()
{
	int i, c;

	if (!verbose)
		return;

	printf("\n");
	printf("-----------------------------------------------------\n");
	printf("Sampling rate ............................ %.2f\n", sampleRate);
	printf("Frames per buffer ........................ %d\n", framesPerBuffer);
	printf("Number of output channels ................ %d\n", numOutChannels);
	printf("Number of RTcmix internal buses .......... %d\n", numInternalBuses);
	printf("Loaded %d scores...\n", numScores);
	for (i = 0; i < numScores; i++)
		printf("   %s\n", scoreNames[i]);
	printf("\n");
	printf("Type 'c', followed by return, to continue...\n");

	while (1) {
		c = getchar();
		if (c == 'c')
			break;
		usleep(100000);
	}
}

int usage()
{
	printf(USAGE_MSG, progname);
	exit(1);
}

int main(int argc, char *argv[])
{
	int i, result;

	progname = strrchr(argv[0], '/');
	if (progname == NULL)
		progname = argv[0];
	else
		progname++;
	if (argc == 1)
		usage();

	for (i = 0; i < MAX_SCORES; i++) {
		scoreNames[i] = NULL;
		scores[i] = NULL;
	}

	for (i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (arg[0] == '-') {
			switch (arg[1]) {
				case 'a':
					if (++i >= argc)
						usage();
					rescaleFactor = atof(argv[i]);
					if (fabs(rescaleFactor) > 2.0)
						rescaleFactor = 2.0;
					break;
				case 'B':
					if (++i >= argc)
						usage();
					framesPerBuffer = atof(argv[i]);
					if (framesPerBuffer < MIN_FRAMES_PER_BUF)
						framesPerBuffer = MIN_FRAMES_PER_BUF;
					break;
				case 'c':
					if (++i >= argc)
						usage();
					numOutChannels = atof(argv[i]);
					if (numOutChannels < 1)
						numOutChannels = 1;
					break;
				case 'h':
					usage();
					break;
				case 'r':
					if (++i >= argc)
						usage();
					sampleRate = atof(argv[i]);
					if (sampleRate < 8000) // be more stringent?
						sampleRate = 8000;
					break;
				case 'v':
					verbose = 1;
					break;
//FIXME: others: call flush after each score? call destroy after each?
				default:
					usage();
			}
		}
		else {
			const char *name = arg;

			/* verify that filename ends in ".sco" */
			char *p = strrchr(name, '.');
			if (p == NULL || strncmp(p, ".sco", 4L)) {
				fprintf(stderr, "Score names must end in \".sco\" (%s).\n", name);
				exit(-1);
			}

			if (numScores < MAX_SCORES) {
				scoreNames[numScores] = name;
				numScores++;
			}
			else {
				fprintf(stderr, "No more than %d scores allowed.\n", MAX_SCORES);
				exit(-1);
			}
		}
	}

	result = initAudio();
	if (result)
		return -1;

	result = initRTcmix();
	if (result)
		return -1;

	result = loadScores();
	if (result)
		return -1;

	printSettings();

	result = playScores();

	for (i = 0; i < numScores; i++)
		if (scores[i])
			free(scores[i]);

	result = deleteRTcmix();

	return deleteAudio();
}

