/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#define DBUG
//#define DENORMAL_CHECK
#include <pthread.h>
#include <sys/resource.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include "RTcmixMain.h"
#include "prototypes.h"
#include <ugens.h>
#include <ug_intro.h>
#include "version.h"
#include "rt.h"
#include "rtdefs.h"
#include "heap.h"
#include "sockdefs.h"
#include "notetags.h"           // contains defs for note-tagging
#include "dbug.h"
#include "InputFile.h"
#include <MMPrint.h>
#include "RTcmix_API.h"

#if defined(EMBEDDEDAUDIO)
#include "sndlibsupport.h"
#include "EmbeddedAudioDevice.h"
#endif

extern "C" {
#ifdef SGI
  void flush_all_underflows_to_zero();
#endif
#ifdef LINUX
  void sigfpe_handler(int sig);
#endif
}


/* ----------------------------------------------------- detect_denormals --- */
/* Unmask "denormalized operand" bit of the x86 FPU control word, so that
   any operations with denormalized numbers will raise a SIGFPE signal,
   and our handler will be called.  NOTE: This is for debugging only!
   This will not tell you how many denormal ops there are, so just because
   the exception is thrown doesn't mean there's a serious problem.  For
   more info, see: http://www.musicdsp.org/files/other001.txt.
*/
#ifdef LINUX
#ifdef DENORMAL_CHECK
static void
detect_denormals()
{
   #include <fpu_control.h>
   int cw = 0;
   _FPU_GETCW(cw);
   cw &= ~_FPU_MASK_DM;
   _FPU_SETCW(cw);
}
#endif /* DENORMAL_CHECK */
#endif /* LINUX */


#ifndef EMBEDDED

/* ----------------------------------------------------------------- main --- */
int
main(int argc, char *argv[], char **env)
{
#ifdef LINUX
 #ifdef DENORMAL_CHECK
   detect_denormals();
 #endif
   signal(SIGFPE, sigfpe_handler);          /* Install signal handler */
#endif /* LINUX */
#ifdef SGI
   flush_all_underflows_to_zero();
#endif

   clear_print();

   {
   RTcmixMain app(argc, argv, env);
   app.run();
   }
   return 0;
}

#else // EMBEDDED

static RTcmixMain *globalApp;

/* ----------------------------------------------------------- RTcmix_init --- */

int
RTcmix_init()
{
	if (globalApp == NULL) {
		clear_print();
		// BGG no argc and argv in max/msp version mm
		globalApp = new RTcmixMain();
		globalApp->run(); // in max/msp this just sets it all up...
	}
	return 0;
}

int
RTcmix_destroy()
{
	delete globalApp;
	globalApp = NULL;
	return 0;
}

static RTcmixBangCallback	sBangCallback = NULL;
static void *				sBangCallbackContext = NULL;

void
RTcmix_setBangCallback(RTcmixBangCallback inBangCallback, void *inContext)
{
	sBangCallback = inBangCallback;
	sBangCallbackContext = inContext;
}

int bang_ready = 0;

// This is called from inTraverse

void checkForBang()
{  
	if (bang_ready == 1) {
		bang_ready = 0;
		if (sBangCallback != NULL) {
			sBangCallback(sBangCallbackContext);
		}
	}
}

static RTcmixValuesCallback sValuesCallback = NULL;
static float sValuesArray[1024];
static void *sValuesCallbackContext;

void
RTcmix_setValuesCallback(RTcmixValuesCallback inValuesCallback, void *inContext)
{
	sValuesCallback = inValuesCallback;
	sValuesCallbackContext = inContext;
}

int vals_ready = 0;
float maxmsp_vals[MAXDISPARGS];

// This is called from inTraverse

void checkForVals()
{  
	if (vals_ready > 0 && sValuesArray != NULL) { // vals_ready will contain how many vals to return
		int nVals = vals_ready;
		vals_ready = 0;
		// Copy into local static array.  This is what gets handed to callback.
		for (int i = 0; i < nVals; i++) sValuesArray[i] = maxmsp_vals[i];
		if (sValuesCallback) {
			sValuesCallback(sValuesArray, nVals, sValuesCallbackContext);
		}
	}
}

static RTcmixPrintCallback sPrintCallback = NULL;
static void *sPrintCallbackContext = NULL;

void RTcmix_setPrintCallback(RTcmixPrintCallback inPrintCallback, void *inContext)
{
	sPrintCallback = inPrintCallback;
	sPrintCallbackContext = inContext;
}

// This is called from inTraverse

void checkForPrint()
{
	if (!is_print_cleared()) {
		const char *printBuf = MMPrint::mm_print_buf;
		if (sPrintCallback)
			sPrintCallback(printBuf, sPrintCallbackContext);
		clear_print();
	}
}

// Currently this does not support resetting the number of busses.

int RTcmix_resetAudio(float sr, int nchans, int vecsize, int recording)
{
	rtcmix_debug(NULL, "RTcmix_resetAudio entered");
	globalApp->close();
	int status = globalApp->resetparams(sr, nchans, vecsize, recording);
	if (status == 0) {
		status = globalApp->resetAudio(sr, nchans, vecsize, recording);
#if defined(MSPAUDIO) || defined(EMBEDDEDAUDIO)
		if (status == 0) {
			rtcmix_debug(NULL, "RTcmix_resetAudio calling RTcmix::startAudio()");
			// There is no separate call to start audio for embedded
			status = globalApp->startAudio(RTcmix::inTraverse, NULL, globalApp);
		}
#endif
	}
	return status;
}

#ifdef IOS

int RTcmix_startAudio()
{
	return globalApp->startAudio(RTcmix::inTraverse, NULL, globalApp);
}

int RTcmix_stopAudio()
{
	return globalApp->stopAudio();
}

#endif // IOS

#if defined(MSPAUDIO)	/* these are entry points used by MAX/MSP only */

#include "Option.h"

void *gMSPAudioState;	// Used by MSPAudioDevice.cpp

void RTcmix_setMSPState(const char *inSpec, void *inState)
{
	Option::device(inSpec);
	gMSPAudioState = inState;
}

#endif

#ifdef MAXMSP

// this is for dynamic loading of RTcmix instruments (for development)
// only one rtcmix~ may be instantiated for this to work
// the scorefile load() system is disabled in rtcmix~
void loadinst(char *dsoname)
{
	// the dsoname should be a fully-qualified pathname to the dynlib
	globalApp->doload(dsoname);
}

void unloadinst()
{
	// it is necessary to unload the dso directly, otherwise the dlopen()
	// system keeps it in memory
	globalApp->unload();
}

#endif	// MAXMSP

#ifdef PD
int pd_rtsetparams(float sr, int nchans, int vecsize, float *mm_inbuf, float *mm_outbuf)
#else
int RTcmix_setparams(float sr, int nchans, int vecsize, int recording, int bus_count)
#endif
{
#if defined(MSPAUDIO)
	globalApp->close();
	globalApp->resetAudio(sr, nchans, vecsize, recording);
#endif
#ifdef PD
	int recording = 1;
	int bus_count = 0;
#endif
	if (bus_count == 0) bus_count = DEFAULT_MAXBUS;
	int status = globalApp->setparams(sr, nchans, vecsize, recording != 0, bus_count);
#if defined(MSPAUDIO) || defined(EMBEDDEDAUDIO)
	if (status == 0) {
		// There is no separate call to start audio for embedded
		status = globalApp->startAudio(RTcmix::inTraverse, NULL, globalApp);
	}
#endif
	return status;
}

#ifdef EMBEDDEDAUDIO

int RTcmix_setAudioBufferFormat(RTcmix_AudioFormat format, int nchans)
{
	int rtcmix_fmt = 0;
	switch (format) {
		case AudioFormat_16BitInt:
			rtcmix_fmt = NATIVE_SHORT_FMT;
			break;
		case AudioFormat_24BitInt:
			rtcmix_fmt = NATIVE_24BIT_FMT;
			break;
		case AudioFormat_32BitInt:
			rtcmix_fmt = NATIVE_32BIT_FMT;
			break;
		case AudioFormat_32BitFloat:
			rtcmix_fmt = NATIVE_FLOAT_FMT;
			break;
		case AudioFormat_32BitFloat_Normalized:
			rtcmix_fmt = NATIVE_FLOAT_FMT;
			rtcmix_fmt |= MUS_NORMALIZED;
			break;
		default:
			rterror("RTcmix_setAudioBufferFormat", "Unknown format");
			return -1;
	}
	// For now, only interleaved audio is allowed.
	rtcmix_fmt |= MUS_INTERLEAVED;
	return SetEmbeddedCallbackAudioFormat(rtcmix_fmt, nchans);
}

int RTcmix_runAudio(void *inAudioBuffer, void *outAudioBuffer, int nframes)
{
	return globalApp->runAudio(inAudioBuffer, outAudioBuffer, nframes);
}

#endif

// these are set from inlets on the rtcmix~ object, using PFields to
// control the Instruments
// rtcmix~ is set to constrain up to a max of 19 inlets for PFields
// iRTCmix can handle up to MAX_INLETS - DAS

float gInletValues[MAX_INLETS];		// used by RTInlinePField.cpp

// New name
void RTcmix_setPField(int inlet, float pval)
{
	if (inlet <= MAX_INLETS) {
		gInletValues[inlet-1] = pval;
	}
	else {
		die("RTcmix_setPField", "exceeded max inlet count [%d]", MAX_INLETS);
	}
}

void pfield_set(int inlet, float pval) { RTcmix_setPField(inlet, pval); }	// UNTIL WE REMOVE THIS FROM IOS VERSION

// This allows a float audio buffer to be directly loaded as input

int RTcmix_setInputBuffer(char *bufname, float *bufstart, int nframes, int nchans, int modtime)
{
// THIS SHOULD BE HANDLED VIA THE PUBLIC FUNCTION
#if defined(MAXMSP)  || defined(IOS)
	float bufferGainScaling = 32767.0f;
#else
	float bufferGainScaling = 1.0f;
#endif
	return (globalApp->setInputBuffer(bufname, bufstart, nframes, nchans, modtime, bufferGainScaling) >= 0) ? 0 : -1;
}

// returns the number of frames in a named buffer
int RTcmix_getBufferFrameCount(char *bufname)
{
	InputFile *input = globalApp->findInput(bufname, NULL);
	if (input != NULL) {
		return input->duration() / input->sampleRate();
	}
	rtcmix_warn("RTcmix_getBufferFrameCount", "there is no buffer named %s", bufname);
	return -1;
}


// returns the number of channels of a named buffer
int RTcmix_getBufferChannelCount(char *bufname)
{
	InputFile *input = globalApp->findInput(bufname, NULL);
	if (input != NULL) {
		return input->channels();
	}
	rtcmix_warn("RTcmix_getBufferChannelCount", "there is no buffer named %s", bufname);
	return -1;
}


// called for the [flush] message; deletes and reinstantiates the rtQueue
// and rtHeap, thus flushing all scheduled events in the future
void RTcmix_flushScore()
{
	globalApp->resetQueueHeap(); // in RTcmixMain.cpp
}


#endif // EMBEDDED

