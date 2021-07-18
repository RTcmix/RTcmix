/* Simple test of using RTcmix from an embedding app, designed to
	facilitate easy memory debugging with the valgrind memcheck tool.
	John Gibson, 7/31/17
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portaudio.h>

#define EMBEDDEDAUDIO
#include <RTcmix_API.h>

#define SAMPLE_RATE 44100.0
#define FRAMES_PER_BUF 512
#define INPUT_CHANS 0
#define OUTPUT_CHANS 2
#define BUS_COUNT 32

/*
	Choose one of these scores, stored in <testScore>, and set
	an appropriate duration for main to wait until the score
	finishes.
*/
float testScoreDuration1 = 2.1; // match <dur> in sco + slop
char *testScore1 = { "WAVETABLE(0, dur=2, amp=5000, freq=440, 0.5)" };

float testScoreDuration2 = 7.0; // match <dur> in sco + slop
char *testScore2 = { " \
	dur = 5.0 \
	notedur = 1.0 \
	amp = 5000 \
	squish = 2 \
	decay = 2.0 \
	for (st = 0; st < dur; st += 0.11) { \
		freq = irand(200, 500) \
		pan = irand(0, 1) \
		STRUM2(st, notedur, amp, freq, squish, decay, pan) \
	} \
" };

float testScoreDuration3 = 5.5; // match <dur> in sco + slop
char *testScore3 = { " \
	dur = 5.0 \
	reset(44100) \
	amp = 6000 \
	env = maketable(\"line\", 1000, 0,0, 1,1, 20,0) \
	for (st = 0; st < dur; st += 0.25) { \
		freq = irand(200, 500) \
		pan = irand(0, 1) \
		WAVETABLE(st, 0.2, amp * env, freq, pan) \
	} \
" };

PaStream *stream = NULL;

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
	return paContinue;
}

int initAudio()
{
	PaError err = Pa_Initialize();
	if (err != paNoError)
		goto error;

	err = Pa_OpenDefaultStream(&stream,
										INPUT_CHANS,
										OUTPUT_CHANS,
										paFloat32,
										SAMPLE_RATE,
										FRAMES_PER_BUF,
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
	int result = RTcmix_init();
	if (result)
		goto error;

	result = RTcmix_setAudioBufferFormat(AudioFormat_32BitFloat_Normalized,
					OUTPUT_CHANS);
	if (result)
		goto error;

	result = RTcmix_setparams(SAMPLE_RATE,
									  OUTPUT_CHANS,
									  FRAMES_PER_BUF,
									  0,	// taking input?
									  BUS_COUNT);
	if (result)
		goto error;

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

void playScore()
{
#if 1
	RTcmix_parseScore(testScore1, strlen(testScore1));
	Pa_Sleep(testScoreDuration1 * 1000); // wait while callback thread runs
#endif

#if 1
	{
		int i;
		for (i = 0; i < 5; i++) {
			RTcmix_parseScore(testScore2, strlen(testScore2));
			Pa_Sleep(5000);
		}
//		Pa_Sleep(testScoreDuration2 * 1000);
	}
#endif

#if 1
	RTcmix_parseScore(testScore1, strlen(testScore1));
	Pa_Sleep((testScoreDuration1 - 5.0) * 1000);
	RTcmix_parseScore(testScore2, strlen(testScore2));
	Pa_Sleep((testScoreDuration2 - 5.0) * 1000);
	RTcmix_parseScore(testScore3, strlen(testScore3));
	Pa_Sleep(testScoreDuration3 * 1000);
#endif
}

int main(int argc, char *argv[])
{
	int result = initAudio();
	if (result)
		return -1;

	result = initRTcmix();
	if (result)
		return -1;

#if 0
	{
		char *buf = malloc(32);
		buf[32] = 0;	// valgrind will catch this
	}
#endif

	playScore();

	result = deleteRTcmix();

	return deleteAudio();
}

