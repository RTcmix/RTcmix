/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */

/* These are the publically-visible API routines for embedded RTcmix platforms */

#ifdef __cplusplus
extern "C" {
#endif
	typedef void (*RTcmixBangCallback)(void *inContext);
	typedef void (*RTcmixValuesCallback)(float *values, int numValues, void *inContext);
	typedef void (*RTcmixPrintCallback)(const char *printBuffer, void *inContext);
	typedef void (*RTcmixFinishedCallback)(long long frameCount, void *inContext);
	void RTcmix_setPrintLevel(int level);
	int RTcmix_init();
	int RTcmix_destroy();
	int RTcmix_setparams(float sr, int nchans, int vecsize, int recording, int bus_count);
	void RTcmix_setBangCallback(RTcmixBangCallback inBangCallback, void *inContext);
	void RTcmix_setValuesCallback(RTcmixValuesCallback inValuesCallback, void *inContext);
	void RTcmix_setPrintCallback(RTcmixPrintCallback inPrintCallback, void *inContext);
	void RTcmix_setFinishedCallback(RTcmixFinishedCallback inFinishedCallback, void *inContext);
#ifdef IOS
	int RTcmix_startAudio();
	int RTcmix_stopAudio();
#endif
	int RTcmix_resetAudio(float sr, int nchans, int vecsize, int recording);
#ifdef EMBEDDEDAUDIO
	// The format of the audio buffers handed to RTcmix via RTcmix_runAudio()
	typedef enum _RTcmix_AudioFormat {
		AudioFormat_16BitInt = 1,				// 16 bit short integer samples
		AudioFormat_24BitInt = 2,				// 24 bit (3-byte) packed integer samples
		AudioFormat_32BitInt = 4,				// 32 bit (4-byte) integer samples
		AudioFormat_32BitFloat_Normalized = 8,	// single-precision float samples, scaled between -1.0 and 1.0
		AudioFormat_32BitFloat = 16				// single-precision float samples, scaled between -32767.0 and 32767.0
	} RTcmix_AudioFormat;
	int RTcmix_setAudioBufferFormat(RTcmix_AudioFormat format, int nchans);
	// Call this to send and receive audio from RTcmix
	int RTcmix_runAudio(void *inAudioBuffer, void *outAudioBuffer, int nframes);
#endif
	int RTcmix_parseScore(char *theBuf, int buflen);
	void RTcmix_flushScore();
	int RTcmix_setInputBuffer(char *bufname, float *bufstart, int nframes, int nchans, int modtime);
	int RTcmix_getBufferFrameCount(char *bufname);
	int RTcmix_getBufferChannelCount(char *bufname);
	void RTcmix_setPField(int inlet, float pval);
	void pfield_set(int inlet, float pval);
#ifdef MAXMSP
	void RTcmix_setMSPState(const char *inSpec, void *inState);
	void loadinst(char *dsoname);
	void unloadinst();
#endif
	void checkForBang();
	void checkForVals();
	void checkForPrint();
	void notifyIsFinished(long long);
#ifdef __cplusplus
}
#endif

