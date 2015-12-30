/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */

/* These are the publically-visible API routines for embedded RTcmix platforms */

extern "C" {
	typedef void (*RTcmixBangCallback)(void *inContext);
	typedef void (*RTcmixValuesCallback)(float *values, int numValues, void *inContext);
	typedef void (*RTcmixPrintCallback)(const char *printBuffer, void *inContext);
	int RTcmix_init();
	int RTcmix_destroy();
	int RTcmix_setparams(float sr, int nchans, int vecsize, int recording, int bus_count);
	void RTcmix_setBangCallback(RTcmixBangCallback inBangCallback, void *inContext);
	void RTcmix_setValuesCallback(RTcmixValuesCallback inValuesCallback, void *inContext);
	void RTcmix_setPrintCallback(RTcmixPrintCallback inPrintCallback, void *inContext);
#if !defined(MAXMSP) && !defined(PD)
	int RTcmix_startAudio();
	int RTcmix_stopAudio();
#endif
	int RTcmix_resetAudio(float sr, int nchans, int vecsize, int recording);
	void RTcmix_flushScore();
	void RTcmix_setPField(int inlet, float pval);
	int RTcmix_setInputBuffer(char *bufname, float *bufstart, int nframes, int nchans, int modtime);
	int RTcmix_getBufferFrameCount(char *bufname);
	int RTcmix_getBufferChannelCount(char *bufname);
	// DAS Still need to rename these
	void pfield_set(int inlet, float pval);
#ifdef MAXMSP
	void RTcmix_setMSPState(const char *inSpec, void *inState);
	void loadinst(char *dsoname);
	void unloadinst();
#endif
	void checkForBang();
	void checkForVals();
	void checkForPrint();
}
