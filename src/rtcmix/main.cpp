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

#include "Option.h"

// BGGx
#define TOTOBJS 178
static RTcmixMain *globalApp[TOTOBJS];

// BGGx -- keep all sets for each instance separate
double OBJgInletValues[TOTOBJS][MAX_INLETS];
double *gInletValues;		// used by RTInlinePField.cpp

// BGG wmm
#include <wmmcontext.h>
wmmcontext theContext[TOTOBJS];

void RTcmix_setPrintLevel(int level)
{
	Option::print(level);
}

/* ----------------------------------------------------------- RTcmix_init --- */

// BGGx -- added objnos
int
RTcmix_init(int objno)
{
//	if (globalApp[objno] == NULL) {
        rtcmix_debug("RTcmix_init", "creating main object %d", objno);
		clear_print();
		// BGG no argc and argv in max/msp version mm

		globalApp[objno]->clear_wmmcontext(objno);
		globalApp[objno]->get_wmmcontext(objno);

		// BGGx set this here for any initialized inlet pfields
		gInletValues = OBJgInletValues[objno];

		globalApp[objno] = new RTcmixMain();
		globalApp[objno]->run(); // in max/msp this just sets it all up...

		globalApp[objno]->set_wmmcontext(objno);
//	}
	return 0;
}

int
RTcmix_destroy(int objno)
{
	globalApp[objno]->get_wmmcontext(objno);
	delete globalApp[objno];
//	globalApp[objno] = NULL;
	// BGGxx
	globalApp[objno]->set_wmmcontext(objno);
	return 0;
}

// BGGxx -- not using these in unity; do check_bang poll instead (see below)
static RTcmixBangCallback	sBangCallback = NULL;
static void *				sBangCallbackContext = NULL;

void
RTcmix_setBangCallback(RTcmixBangCallback inBangCallback, void *inContext, int objno)
{
	sBangCallback = inBangCallback;
	sBangCallbackContext = inContext;
}

int bang_ready = 0;

// BGGx -- don't do the bang callback scheme below unity_checkForBang()
//		the reason has to do with the context-switching:  for a score to be
//		sent and parsed, the wmmcontext_busy flag has to be 0 to allow it.
//		BUT if it is in a callback sequence of function calls from intraverse,
//		then it will still be 1.  I thought about temporarily resetting it
//		but this could allow another RTcmix to sneak in with its context.
int unity_checkForBang(int objno)
{
	int retval;

	retval = 0;
	globalApp[objno]->get_wmmcontext(objno);
	if (bang_ready == 1) {
		bang_ready = 0;
		retval = 1;
	}
	globalApp[objno]->set_wmmcontext(objno);
	return retval;
}


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

// BGGx -- do the polling for vals (see above note for unity_checkForBang)
int unity_checkForVals(float *thevals, int objno)
{
	int retval;

	globalApp[objno]->get_wmmcontext(objno);
	if (vals_ready > 0) {
		retval = vals_ready;  // vals_ready has the # of vals
		for (int i = 0; i < retval; i++)
			thevals[i] = maxmsp_vals[i];
		vals_ready = 0;
	} else {
		retval = 0;
	}
	globalApp[objno]->set_wmmcontext(objno);
	return retval;
}

// This is called from inTraverse

void checkForVals()
{  
	if (vals_ready > 0) { // vals_ready will contain how many vals to return
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

// BGGx -- this is to return our 'lines' from the prinbuf because
//		stoopid C# won't allow me to advance a pointer to a string

char *pcheckptr;
int firstprint;

// BGGx -- do the polling for printing (see above note for unity_checkForBang)
int unity_checkForPrint(char *pbuf, int objno)
{
	char *printBuf;
	int retval;

	globalApp[objno]->get_wmmcontext(objno);
	if (!is_print_cleared()) {
		if (firstprint == 0) {
			pcheckptr = MMPrint::mm_print_buf;
			firstprint = 1;
		}

		printBuf = pcheckptr;

		if (strlen(printBuf) > 0) {
			int i,j;
			if (strlen(printBuf) > 0) {
				// BGGx -- unity uses unicode, hence the += 2
				for (i = 0, j = 0; i < strlen(printBuf); i++, j += 2)
					pbuf[j] = printBuf[i];

				pcheckptr += (i+1);
			}
			retval = 1;
		} else {
			clear_print();
			firstprint = 0;
			retval = 0;
		}
   } else {
		retval = 0;
	}
	globalApp[objno]->set_wmmcontext(objno);
	return retval;
}

void RTcmix_setPrintCallback(RTcmixPrintCallback inPrintCallback, void *inContext)
{
	sPrintCallback = inPrintCallback;
	sPrintCallbackContext = inContext;
}

static RTcmixFinishedCallback sFinishedCallback = NULL;
static void *sFinishedCallbackContext = NULL;

void RTcmix_setFinishedCallback(RTcmixFinishedCallback inFinishedCallback, void *inContext)
{
	sFinishedCallback = inFinishedCallback;
	sFinishedCallbackContext = inContext;
}

// These are called from inTraverse

void checkForPrint()
{
	if (!is_print_cleared()) {
		const char *printBuf = MMPrint::mm_print_buf;
		if (sPrintCallback)
			sPrintCallback(printBuf, sPrintCallbackContext);
		clear_print();
	}
}

void notifyIsFinished(long long endFrame)
{
	if (sFinishedCallback) {
		sFinishedCallback(endFrame, sFinishedCallbackContext);
	}
}

// Currently this does not support resetting the number of busses.

int RTcmix_resetAudio(float sr, int nchans, int vecsize, int recording, int objno)
{
	globalApp[objno]->get_wmmcontext(objno);
	rtcmix_debug(NULL, "RTcmix_resetAudio entered");
	globalApp[objno]->close();
	int status = globalApp[objno]->resetparams(sr, nchans, vecsize, recording);
	if (status == 0) {
		status = globalApp[objno]->resetAudio(sr, nchans, vecsize, recording);
#if defined(MSPAUDIO) || defined(EMBEDDEDAUDIO)
		if (status == 0) {
			rtcmix_debug(NULL, "RTcmix_resetAudio calling RTcmix::startAudio()");
			// There is no separate call to start audio for embedded
			status = globalApp[objno]->startAudio(RTcmix::inTraverse, NULL, globalApp);
		}
#endif
	}
	globalApp[objno]->set_wmmcontext(objno);
	return status;
}

#ifdef IOS

int RTcmix_startAudio(int objno)
{
	return globalApp[objno]->startAudio(RTcmix::inTraverse, NULL, globalApp[objno]);
}

int RTcmix_stopAudio(int objno)
{
	return globalApp[objno]->stopAudio();
}

#endif // IOS

#ifdef MAXMSP

// this is for dynamic loading of RTcmix instruments (for development)
// only one rtcmix~ may be instantiated for this to work
// the scorefile load() system is disabled in rtcmix~
void loadinst(char *dsoname)
{
	// the dsoname should be a fully-qualified pathname to the dynlib
	globalApp[objno]->doload(dsoname);
}

void unloadinst()
{
	// it is necessary to unload the dso directly, otherwise the dlopen()
	// system keeps it in memory
	globalApp[objno]->unload();
}

#endif	// MAXMSP

int RTcmix_setparams(float sr, int nchans, int vecsize, int recording, int bus_count, int objno)
{
	globalApp[objno]->get_wmmcontext(objno);

#warning is it still necessary to close and reset audio in setparams?
#if defined(MSPAUDIO)
	globalApp[objno]->close();
	globalApp[objno]->resetAudio(sr, nchans, vecsize, recording);
#endif
	if (bus_count == 0) bus_count = DEFAULT_MAXBUS;
	int status = globalApp[objno]->setparams(sr, nchans, vecsize, recording != 0, bus_count);
#if defined(MSPAUDIO) || defined(EMBEDDEDAUDIO)
	if (status == 0) {
		// There is no separate call to start audio for embedded
		status = globalApp[objno]->startAudio(RTcmix::inTraverse, NULL, globalApp[objno]);
	}
#endif

	globalApp[objno]->set_wmmcontext(objno);
	return status;
}

#ifdef EMBEDDEDAUDIO

int RTcmix_setAudioBufferFormat(RTcmix_AudioFormat format, int nchans, int objno)
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
			return die("RTcmix_setAudioBufferFormat", "Unknown format");
	}
	// For now, only interleaved audio is allowed.
	rtcmix_fmt |= MUS_INTERLEAVED;
	return SetEmbeddedCallbackAudioFormat(rtcmix_fmt, nchans);
}

// BGGx -- for unity
float tinputbuf[2048];	// so we only have to pass one buffer ptr in unity

int RTcmix_runAudio(void *inAudioBuffer, void *outAudioBuffer, int nframes, int objno)
{
	// BGGx
	int retval;

	if (inAudioBuffer != NULL) {
		for (int i = 0; i < 2048; i++)
			tinputbuf[i] = ((float *)inAudioBuffer)[i];
	} else {
		bzero(tinputbuf, sizeof(float) * 2048);
	}

	globalApp[objno]->get_wmmcontext(objno);

	// BGGx set this prior to the runAudio
	gInletValues = OBJgInletValues[objno];

	retval = globalApp[objno]->runAudio(tinputbuf, outAudioBuffer, nframes);
	globalApp[objno]->set_wmmcontext(objno);
	return(retval);
}

#endif

// these are set from inlets on the rtcmix~ object, using PFields to
// control the Instruments
// rtcmix~ is set to constrain up to a max of 19 inlets for PFields
// iRTCmix can handle up to MAX_INLETS - DAS

// BGGx -- OBJgInletValues  and gInletValues declared up above (they are
// used in the RTcmix_runAudio() function(

// New name
void RTcmix_setPField(int inlet, float pval, int objno)
{
	// BGGx -- no context necessary; just make an entry into the correct
	// OBJgInletValues slot.  This is then copied to gInletValues just
	// before running audio, so the correct value will be used
	if (inlet <= MAX_INLETS) {
		OBJgInletValues[objno][inlet-1] = pval;
	}
	else {
		die("RTcmix_setPField", "exceeded max inlet count [%d]", MAX_INLETS);
	}
}

// BGGx
// void pfield_set(int inlet, float pval) { RTcmix_setPField(inlet, pval); }	// UNTIL WE REMOVE THIS FROM IOS VERSION

// This allows a float audio buffer to be directly loaded as input

int RTcmix_setInputBuffer(char *bufname, float *bufstart, int nframes, int nchans, int modtime, int objno)
{
// THIS SHOULD BE HANDLED VIA THE PUBLIC FUNCTION
#if defined(MAXMSP) || defined(PD) || defined(IOS)
	float bufferGainScaling = 32767.0f;
#else
	float bufferGainScaling = 1.0f;
#endif
	return (globalApp[objno]->setInputBuffer(bufname, bufstart, nframes, nchans, modtime, bufferGainScaling) >= 0) ? 0 : -1;
}

// returns the number of frames in a named buffer
int RTcmix_getBufferFrameCount(char *bufname, int objno)
{
	InputFile *input = globalApp[objno]->findInput(bufname, NULL);
	if (input != NULL) {
		return input->duration() / input->sampleRate();
	}
	rtcmix_warn("RTcmix_getBufferFrameCount", "there is no buffer named %s", bufname);
	return -1;
}


// returns the number of channels of a named buffer
int RTcmix_getBufferChannelCount(char *bufname, int objno)
{
	InputFile *input = globalApp[objno]->findInput(bufname, NULL);
	if (input != NULL) {
		return input->channels();
	}
	rtcmix_warn("RTcmix_getBufferChannelCount", "there is no buffer named %s", bufname);
	return -1;
}


// called for the [flush] message; deletes and reinstantiates the rtQueue
// and rtHeap, thus flushing all scheduled events in the future
void RTcmix_flushScore(int objno)
{
	globalApp[objno]->get_wmmcontext(objno);
	globalApp[objno]->resetQueueHeap(); // in RTcmixMain.cpp
	globalApp[objno]->set_wmmcontext(objno);
}


// BGGx
extern "C" int RTcmix_parseScore(char *thebuf, int buflen);

int unity_parse_score(char *buf, int len, int objno)
{
   int retval;

   globalApp[objno]->get_wmmcontext(objno);
   retval = RTcmix_parseScore(buf, len);
   globalApp[objno]->set_wmmcontext(objno);

   return retval;
}


#endif // EMBEDDED

