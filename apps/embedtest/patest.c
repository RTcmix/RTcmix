/* Simple example of using PortAudio to play noise.
	Mostly swiped from PortAudio distro.
	John Gibson, 7/31/17
*/
#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>

#define SAMPLE_RATE 44100.0
#define FRAMES_PER_BUF 256
#define INPUT_CHANS 0
#define OUTPUT_CHANS 2
#define NUM_SECONDS 10

PaStream *stream = NULL;
float amp = 0.05;

float myrand()
{
	int r = rand();
	return (((float) r / RAND_MAX) * 2.0) - 1.0;	// scale to [-1,1]
}

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
	float *out = (float *) output;
	unsigned long i;

	for (i = 0; i < frames; i++) {
		float sig = myrand() * amp;
		*out++ = sig;
		*out++ = sig;	// same for each chan
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

int main(int argc, char *argv[])
{
	int result = initAudio();
	if (result)
		return -1;

	// wait while callback thread runs
	Pa_Sleep(NUM_SECONDS * 1000);

	return deleteAudio();
}

