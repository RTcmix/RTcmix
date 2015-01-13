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
/* ----------------------------------------------------------- RTcmix_init --- */
static RTcmixMain *app;

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
#ifdef IOS
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
	// DAS These are called from inTraverse()
	void checkForBang();
	void checkForVals();
	void checkForPrint();
}

int
RTcmix_init()	// BGG mm -- now called this for max/msp
{
	clear_print();

// BGG no argc and argv in max/msp version mm
   app = new RTcmixMain();
   app->run(); // in max/msp this just sets it all up...

   return 0;
}

int
RTcmix_destroy()
{
	delete app;
	app = NULL;
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
	app->close();
	int status = app->resetparams(sr, nchans, vecsize, recording);
	if (status == 0) {
		status = app->resetAudio(sr, nchans, vecsize, recording);
#if defined(MSP_AUDIO_DEVICE)
		if (status == 0) {
			rtcmix_debug(NULL, "RTcmix_resetAudio calling RTcmix::startAudio()");
			// For now, there is no separate call to start audio in rtcmix~
			status = app->startAudio(RTcmix::inTraverse, NULL, app);
		}
#endif
	}
	return status;
}

#ifdef IOS
	
int RTcmix_startAudio()
{
	return app->startAudio(RTcmix::inTraverse, NULL, app);
}

int RTcmix_stopAudio()
{
	return app->stopAudio();
}

#endif // IOS

#if defined(MAXMSP)	/* these are entry points used by MAX/MSP only */

#include "Option.h"

void *gMSPAudioState;	// Used by MSPAudioDevice.cpp

void RTcmix_setMSPState(const char *inSpec, void *inState)
{
	Option::device(inSpec);
	gMSPAudioState = inState;
}

// this is for dynamic loading of RTcmix instruments (for development)
// only one rtcmix~ may be instantiated for this to work
// the scorefile load() system is disabled in rtcmix~
void loadinst(char *dsoname)
{
	// the dsoname should be a fully-qualified pathname to the dynlib
	app->doload(dsoname);
}

void unloadinst()
{
	// it is necessary to unload the dso directly, otherwise the dlopen()
	// system keeps it in memory
	app->unload();
}

#endif	// MAXMSP

#ifdef PD
int pd_rtsetparams(float sr, int nchans, int vecsize, float *mm_inbuf, float *mm_outbuf)
#else
int RTcmix_setparams(float sr, int nchans, int vecsize, int recording, int bus_count)	// DAS WAS mm_rtsetparams
#endif
{
#if defined(MSP_AUDIO_DEVICE)
	app->close();
	app->resetAudio(sr, nchans, vecsize, recording);
#endif
#ifdef PD
	int recording = 1;
	int bus_count = 0;
#endif
	if (bus_count == 0) bus_count = DEFAULT_MAXBUS;
	int status = app->setparams(sr, nchans, vecsize, recording != 0, bus_count);
#if defined(MSP_AUDIO_DEVICE)
	if (status == 0) {
		// For now, there is no separate call to start audio in rtcmix~
		status = app->startAudio(RTcmix::inTraverse, NULL, app);
	}
#endif
	return status;
}


// these are set from inlets on the rtcmix~ object, using PFields to
// control the Instruments
// rtcmix~ is set to constrain up to a max of 19 inlets for PFields
// iRTCmix can handle up to MAX_INLETS - DAS

float gInletValues[MAX_INLETS];

void pfield_set(int inlet, float pval)
{
	if (inlet <= MAX_INLETS) {
		gInletValues[inlet-1] = pval;
	}
	else {
		die("pfield_set", "exceeded max inlet count [%d]", MAX_INLETS);
	}
}

// New name
void RTcmix_setPField(int inlet, float pval)
{
	pfield_set(inlet, pval);
}

// This allows a float audio buffer to be directly loaded as input

int RTcmix_setInputBuffer(char *bufname, float *bufstart, int nframes, int nchans, int modtime)
{
	return (app->setInputBuffer(bufname, bufstart, nframes, nchans, modtime) >= 0) ? 0 : -1;
}

#ifdef OPENFRAMEWORKS

// this is used for OpenFrameworks; will read a soundfile into a named
// buffer for use by rtinput("MMBUF", "namedbuffer")
void OF_buffer_load_set(char *filename, char *bufname, float insk, float dur)
{
}

#endif // OPENFRAMEWORKS


// returns the number of frames in a named buffer
int RTcmix_getBufferFrameCount(char *bufname)
{
	InputFile *input = app->findInput(bufname, NULL);
	if (input != NULL) {
		return input->duration() / input->sampleRate();
	}
	rtcmix_warn("RTcmix_getBufferFrameCount", "there is no buffer named %s", bufname);
	return -1;
}


// returns the number of channels of a named buffer
int RTcmix_getBufferChannelCount(char *bufname)
{
	InputFile *input = app->findInput(bufname, NULL);
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
	app->resetQueueHeap(); // in RTcmixMain.cpp
}


#endif // EMBEDDED

