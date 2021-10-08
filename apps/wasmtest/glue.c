#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <emscripten.h>

#define EMBEDDEDAUDIO
#include <RTcmix_API.h>

#define BUS_COUNT 32

#define _FRAME_SIZE 1024
EMSCRIPTEN_KEEPALIVE
const int FRAME_SIZE = _FRAME_SIZE;

EMSCRIPTEN_KEEPALIVE
#define _NUM_CHANNELS 2
const int NUM_CHANNELS = _NUM_CHANNELS;

float interleaved[_FRAME_SIZE * _NUM_CHANNELS];

EMSCRIPTEN_KEEPALIVE
float output[_FRAME_SIZE * _NUM_CHANNELS];

int finished;

void on_finish(long long frameCount, void *inContext) {
	finished = 1;
}

EMSCRIPTEN_KEEPALIVE
void setup(int sample_rate) {
	RTcmix_init();
	RTcmix_setAudioBufferFormat(AudioFormat_32BitFloat_Normalized, NUM_CHANNELS);
	RTcmix_setFinishedCallback(on_finish, NULL);
	RTcmix_setparams(sample_rate, NUM_CHANNELS, FRAME_SIZE, 0, BUS_COUNT);
}

EMSCRIPTEN_KEEPALIVE
void load_score(char *score) {
	finished = 0;
    RTcmix_parseScore(score, strlen(score));
}

EMSCRIPTEN_KEEPALIVE
int process() {
	if (finished) {
		return 0;
	}
	RTcmix_runAudio(NULL, interleaved, FRAME_SIZE);
	// Deinterleave channels for Web Audio API.
	for (int frame = 0; frame < FRAME_SIZE; frame++) {
		for (int channel = 0; channel < NUM_CHANNELS; channel++) {
			output[channel * FRAME_SIZE + frame] = interleaved[frame * NUM_CHANNELS + channel];
		}
	}
	return 1;
}
