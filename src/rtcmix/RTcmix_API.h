/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */

/* These are the publically-visible API routines for embedded RTcmix platforms */

// BGGx
// added an "objno" parameter to many of the embedded API calls

extern "C" {
	// BGGxx
	__declspec(dllexport) int heyho();

	typedef void (*RTcmixBangCallback)(void *inContext);
	typedef void (*RTcmixValuesCallback)(float *values, int numValues, void *inContext);
	typedef void (*RTcmixPrintCallback)(const char *printBuffer, void *inContext);
	__declspec(dllexport) int RTcmix_init(int objno);
	__declspec(dllexport) int RTcmix_destroy(int objno);
	__declspec(dllexport) int RTcmix_setparams(float sr, int nchans, int vecsize, int recording, int bus_count, int objno);
	__declspec(dllexport) void RTcmix_setBangCallback(RTcmixBangCallback inBangCallback, void *inContext);
	__declspec(dllexport) void RTcmix_setValuesCallback(RTcmixValuesCallback inValuesCallback, void *inContext);
	__declspec(dllexport) void RTcmix_setPrintCallback(RTcmixPrintCallback inPrintCallback, void *inContext);
#ifdef IOS
	int RTcmix_startAudio(int objno);
	int RTcmix_stopAudio(int objno);
#endif
	__declspec(dllexport) int RTcmix_resetAudio(float sr, int nchans, int vecsize, int recording, int objno);
#ifdef EMBEDDEDAUDIO
	// The format of the audio buffers handed to RTcmix via RTcmix_runAudio()
	typedef enum _RTcmix_AudioFormat {
		AudioFormat_16BitInt = 1,				// 16 bit short integer samples
		AudioFormat_24BitInt = 2,				// 24 bit (3-byte) packed integer samples
		AudioFormat_32BitInt = 4,				// 32 bit (4-byte) integer samples
		AudioFormat_32BitFloat_Normalized = 8,	// single-precision float samples, scaled between -1.0 and 1.0
		AudioFormat_32BitFloat = 16				// single-precision float samples, scaled between -32767.0 and 32767.0
	} RTcmix_AudioFormat;
	__declspec(dllexport) int RTcmix_setAudioBufferFormat(RTcmix_AudioFormat format, int nchans, int objno);
	// Call this to send and receive audio from RTcmix
	__declspec(dllexport) int RTcmix_runAudio(void *inAudioBuffer, void *outAudioBuffer, int nframes, int objno);
#endif

// BGGx
//	int RTcmix_parseScore(char *theBuf, int buflen);
	__declspec(dllexport) int unity_parse_score(char *theBuf, int buflen, int objno);
	__declspec(dllexport) int unity_checkForBang(int objno);
	__declspec(dllexport) int unity_checkForVals(float *vals, int objno);
	__declspec(dllexport) int unity_checkForPrint(char *pbuf, int objno);

	__declspec(dllexport) void RTcmix_flushScore(int objno);
	__declspec(dllexport) int RTcmix_setInputBuffer(char *bufname, float *bufstart, int nframes, int nchans, int modtime, int objno);
	__declspec(dllexport) int RTcmix_getBufferFrameCount(char *bufname, int objno);
	__declspec(dllexport) int RTcmix_getBufferChannelCount(char *bufname, int objno);
	__declspec(dllexport) void RTcmix_setPField(int inlet, float pval, int objno);
	__declspec(dllexport) void pfield_set(int inlet, float pval);
#ifdef MAXMSP
	void loadinst(char *dsoname);
	void unloadinst();
#endif
	__declspec(dllexport) void checkForBang();
	__declspec(dllexport) void checkForVals();
	__declspec(dllexport) void checkForPrint();

// BGGx
	__declspec(dllexport) int check_context();
}
