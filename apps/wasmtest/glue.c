#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <emscripten.h>

#define EMBEDDEDAUDIO
#include <RTcmix_API.h>

#define BUS_COUNT 32

#define _BLOCK_SIZE 1024
EMSCRIPTEN_KEEPALIVE
const int BLOCK_SIZE = _BLOCK_SIZE;

EMSCRIPTEN_KEEPALIVE
float output[_BLOCK_SIZE];

EMSCRIPTEN_KEEPALIVE
void setup(int sample_rate) {
	RTcmix_init();
	RTcmix_setAudioBufferFormat(AudioFormat_32BitFloat_Normalized, 1);
	RTcmix_setparams(sample_rate, 1, BLOCK_SIZE, 0, BUS_COUNT);
}

EMSCRIPTEN_KEEPALIVE
void load_score(char *score) {
    RTcmix_parseScore(score, strlen(score));
}

EMSCRIPTEN_KEEPALIVE
void process() {
	RTcmix_runAudio(NULL, output, BLOCK_SIZE);
}
