/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */

/* These are the publically-visible API routines for embedded RTcmix platforms */

// BGGx
// added an "objno" parameter to many of the embedded API calls

#ifdef __cplusplus
extern "C" {
#endif
	typedef void (*RTcmixBangCallback)(void *inContext);
	typedef void (*RTcmixValuesCallback)(float *values, int numValues, void *inContext);
	typedef void (*RTcmixPrintCallback)(const char *printBuffer, void *inContext);
	typedef void (*RTcmixFinishedCallback)(long long frameCount, void *inContext);
	void RTcmix_setPrintLevel(int level);
	int RTcmix_init(int objno);
	int RTcmix_destroy(int objno);
	int RTcmix_setparams(float sr, int nchans, int vecsize, int recording, int bus_count, int objno);
	void RTcmix_setBangCallback(RTcmixBangCallback inBangCallback, void *inContext);
	void RTcmix_setValuesCallback(RTcmixValuesCallback inValuesCallback, void *inContext);
	void RTcmix_setPrintCallback(RTcmixPrintCallback inPrintCallback, void *inContext);
	void RTcmix_setFinishedCallback(RTcmixFinishedCallback inFinishedCallback, void *inContext);
#ifdef IOS
	int RTcmix_startAudio(int objno);
	int RTcmix_stopAudio(int objno);
#endif
	int RTcmix_resetAudio(float sr, int nchans, int vecsize, int recording, int objno);
#ifdef EMBEDDEDAUDIO
	// The format of the audio buffers handed to RTcmix via RTcmix_runAudio()
	typedef enum _RTcmix_AudioFormat {
		AudioFormat_16BitInt = 1,				// 16 bit short integer samples
		AudioFormat_24BitInt = 2,				// 24 bit (3-byte) packed integer samples
		AudioFormat_32BitInt = 4,				// 32 bit (4-byte) integer samples
		AudioFormat_32BitFloat_Normalized = 8,	// single-precision float samples, scaled between -1.0 and 1.0
		AudioFormat_32BitFloat = 16				// single-precision float samples, scaled between -32767.0 and 32767.0
	} RTcmix_AudioFormat;
	int RTcmix_setAudioBufferFormat(RTcmix_AudioFormat format, int nchans, int objno);
	// Call this to send and receive audio from RTcmix
	int RTcmix_runAudio(void *inAudioBuffer, void *outAudioBuffer, int nframes, int objno);
#endif

// BGGx
//	int RTcmix_parseScore(char *theBuf, int buflen);
	int unity_parse_score(char *theBuf, int buflen, int objno);
	int unity_checkForBang(int objno);
	int unity_checkForVals(float *vals, int objno);
	int unity_checkForPrint(char *pbuf, int objno);

	void RTcmix_flushScore(int objno);
	int RTcmix_setInputBuffer(char *bufname, float *bufstart, int nframes, int nchans, int modtime, int objno);
	int RTcmix_getBufferFrameCount(char *bufname, int objno);
	int RTcmix_getBufferChannelCount(char *bufname, int objno);
	void RTcmix_setPField(int inlet, float pval, int objno);
	void pfield_set(int inlet, float pval);
#ifdef MAXMSP
	void loadinst(char *dsoname);
	void unloadinst();
#endif
	void checkForBang();
	void checkForVals();
	void checkForPrint();
	void notifyIsFinished(long long);
    // BGGx
    int check_context();
#ifdef __cplusplus
}
#endif

