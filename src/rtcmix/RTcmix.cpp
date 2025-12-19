// OK -- this is now RTcmix.C, hacked from the original "main.C"
// 	in standalone RTcmix.  The idea is to make this a compile-in
//	imbRTcmix.o lib that can be called from within other main()
//	progs.  -- BGG 11/2002

/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include <RTcmix.h>

//#define DBUG
//#define DENORMAL_CHECK
#include <pthread.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <math.h>

#include "prototypes.h"
#include "InputFile.h"
#include <ugens.h>
#include <RTOption.h>
#include "handle.h"
#include <ug_intro.h>
#include <AudioDevice.h>
#ifdef MULTI_THREAD
#include "TaskManager.h"
#endif
#include "rt.h"
#include "heap.h"
#include "maxdispargs.h"
#include "dbug.h"
#include "globals.h"
#ifdef EMBEDDED
#include "../parser/rtcmix_parse.h"
#define INTERACTIVE_DEFAULT 0
#else
#define INTERACTIVE_DEFAULT 1
#endif

#define TLEN 20  /* maximum number of time/tempo pairs */
#define TLENP 21

// This is declared (still) in globals.h for use in gen routines.

FILE *	infile_desc[MAX_INFILE_DESC + 1];

extern "C" {
#ifdef SGI
  void flush_all_underflows_to_zero();
#endif
#ifdef LINUX
  void sigfpe_handler(int sig);
#endif
}

// Static RTcmix member initialization

int				RTcmix::NCHANS 			= 2;
int				RTcmix::sBufferFrameCount = 0;
int				RTcmix::audioNCHANS 	= 0;
float			RTcmix::sSamplingRate	= 0.0;
bool			RTcmix::runToOffset		= false;
float   		RTcmix::bufTimeOffset	= 0.0;
FRAMETYPE		RTcmix::bufStartSamp 	= 0;

int				RTcmix::rtInteractive = INTERACTIVE_DEFAULT;
int             RTcmix::rtUsingOSC = 0;

int				RTcmix::rtsetparams_called = 0; // will call at object instantiation, though
int				RTcmix::audioLoopStarted = 0;
int				RTcmix::audio_config 	= 1;
FRAMETYPE		RTcmix::elapsed 		= 0;
RTstatus		RTcmix::run_status      = RT_GOOD;
AudioDevice *	RTcmix::audioDevice     = NULL;
pthread_mutex_t RTcmix::audio_config_lock = PTHREAD_MUTEX_INITIALIZER;

heap *			RTcmix::rtHeap			= NULL;
RTQueue *		RTcmix::rtQueue			= NULL;
rt_item *		RTcmix::rt_list 		= NULL;

int				RTcmix::output_data_format 		= -1;
int				RTcmix::output_header_type 		= -1;
int				RTcmix::normalize_output_floats	= 0;
int				RTcmix::is_float_format 		= 0;
char *			RTcmix::rtoutsfname 			= NULL;

double           RTcmix::xtime[TLENP];
double           RTcmix::tempovals[TLENP];
double           RTcmix::rxtime[TLENP];
double           RTcmix::accel[TLENP];
double           RTcmix::BASIS                   = 60.;
short           RTcmix::tempo_set               = 0;
short           RTcmix::numTimePoints           = 0;

BufPtr *		RTcmix::audioin_buffer = NULL;    /* input from ADC, not file */
BufPtr *		RTcmix::aux_buffer = NULL;
BufPtr *		RTcmix::out_buffer = NULL;

bool		RTcmix::rtrecord 	= false;		// indicates reading from audio device
int			RTcmix::rtfileit 	= 0;		// signal writing to soundfile
int			RTcmix::rtoutfile 	= 0;

InputFile *	RTcmix::inputFileTable = NULL;
long		RTcmix::max_input_fds = 0;
int			RTcmix::last_input_index = -1;

pthread_rwlock_t RTcmix::bus_playlist_rwlock = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t RTcmix::bus_config_rwlock = PTHREAD_RWLOCK_INITIALIZER;

short *			RTcmix::AuxToAuxPlayList = NULL;
short *			RTcmix::ToOutPlayList = NULL;
short *			RTcmix::ToAuxPlayList = NULL;

#ifdef MULTI_THREAD
//pthread_mutex_t RTcmix::aux_buffer_lock = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t RTcmix::out_buffer_lock = PTHREAD_MUTEX_INITIALIZER;
TaskManager *	RTcmix::taskManager = NULL;
std::vector<RTcmix::MixData> RTcmix::mixVectors[RT_THREAD_COUNT];
#endif

std::vector<RTcmix::CallbackInfo> RTcmix::audioStartCallbacks;
std::vector<RTcmix::CallbackInfo> RTcmix::audioStopCallbacks;
std::vector<RTcmix::CallbackInfo> RTcmix::destroyCallbacks;

// Bus config state

BusQueue *		RTcmix::Inst_Bus_Config;
Locked<Bool>	RTcmix::Bus_Config_Status(NO);
int				RTcmix::busCount = DEFAULT_MAXBUS;
BusConfig *		RTcmix::BusConfigs = NULL;

// Function registry
FunctionEntry *	RTcmix::_functionRegistry = NULL;

// Function table state
struct _func *	RTcmix::_func_list = NULL;


/* --------------------------------------------------------- init_options --- */
void
RTcmix::init_options(bool fromMain, const char *defaultDSOPath)
{
	rtcmix_debug(NULL, "RTcmix::init_options entered");
	RTOption::init();
	if (defaultDSOPath && defaultDSOPath[0])
		RTOption::dsoPathPrepend(defaultDSOPath);
	
	if (fromMain) {
#ifndef EMBEDDED
		RTOption::readConfigFile(RTOption::rcName());
		RTOption::exitOnError(true); // we do this no matter what is in config file
#else
		RTOption::exitOnError(false);
        RTOption::print(6);
#endif
        setInteractive(false);
	}
	else {
		setSR(44100.0); // what the heck...
		RTOption::print(0);
		RTOption::reportClipping(false);
	}
	
	setRTBUFSAMPS((int) RTOption::bufferFrames());  /* modifiable with rtsetparams */

	if (RTOption::autoLoad()) {
		const char *dsoPath = RTOption::dsoPath();
		if (strlen(dsoPath) == 0)
			registerDSOs(SHAREDLIBDIR);
		else
			registerDSOs(dsoPath);
	}
}

/* --------------------------------------------------------- init_globals --- */
void
RTcmix::init_globals()
{
   rtcmix_debug(NULL, "RTcmix::init_globals entered");
   rtHeap = new heap;
   rtQueue = new RTQueue[busCount*3];
#ifdef MULTI_THREAD
   taskManager = new TaskManager;
    for (int i = 0; i < RT_THREAD_COUNT; ++i) {
        mixVectors[i].reserve(busCount);
    }
#endif
	BusConfigs = new BusConfig[busCount];
	AuxToAuxPlayList = new short[busCount];
	ToOutPlayList = new short[busCount];
	ToAuxPlayList = new short[busCount];
   for (int i = 0; i < busCount; i++) {
      AuxToAuxPlayList[i] = -1; /* The playback order for AUX buses */
      ToOutPlayList[i] = -1;    /* The playback order for AUX buses */
      ToAuxPlayList[i] =-1;     /* The playback order for AUX buses */
   }
#ifndef EMBEDDED
	max_input_fds = sysconf(_SC_OPEN_MAX);
	if (max_input_fds == -1)	// call failed
		max_input_fds = 128;		// what we used to hardcode
	else
		max_input_fds -= RESERVE_INPUT_FDS;
#else
	// BGGx -- the above doesn't work for rtcmix~ on Big Sur and
	// following OSes.  I'm reverting to our older hard-coded number,
	// which seems to work fine
	max_input_fds = 128;
#endif
	inputFileTable = new InputFile[max_input_fds];
	last_input_index = -1;
	
   init_buf_ptrs();
}

void
RTcmix::free_globals()
{
	rtcmix_debug(NULL, "RTcmix::free_globals entered");
    callDestroyCallbacks();
	free_buffers();
	free_bus_config();
	freefuncs();
    clearRtInstList();
	delete [] rtQueue;
	rtQueue = NULL;
	delete rtHeap;
	rtHeap = NULL;
	delete [] inputFileTable;
	inputFileTable = NULL;
	
	delete [] AuxToAuxPlayList;
	AuxToAuxPlayList = NULL;
	delete [] ToAuxPlayList;
	ToAuxPlayList = NULL;
	delete [] ToOutPlayList;
	ToAuxPlayList = NULL;
	delete [] BusConfigs;
	BusConfigs = NULL;
	
	// Reset state of all global vars
    BASIS                   = 0;
    tempo_set               = 0;
    numTimePoints           = 0;
	runToOffset				= false;
	bufTimeOffset			= 0.0;
	rtsetparams_called 		= 0;
	audioLoopStarted 		= 0;
	audio_config 			= YES;
	elapsed 				= 0;
	run_status      		= RT_GOOD;
	rtrecord 				= false;
	rtfileit 				= 0;
	rtoutfile 				= 0;
	output_data_format 		= -1;
	output_header_type 		= -1;
	
#ifdef MULTI_THREAD
	delete taskManager;
	taskManager = NULL;
	InputFile::destroyConversionBuffers();
#endif

}

/* ----------------------------------------------------- detect_denormals --- */
/* Unmask "denormalized operand" bit of the x86 FPU control word, so that
   any operations with denormalized numbers will raise a SIGFPE signal,
   and our handler will be called.  NOTE: This is for debugging only!
   This will not tell you how many denormal ops there are, so just because
   the exception is thrown doesn't mean there's a serious problem.  For
   more info, see: http://www.smartelectronix.com/musicdsp/text/other001.txt.
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

//  The RTcmix constructor with default SR, NCHANS, and RTBUFSAMPS
RTcmix::RTcmix() 
{
	init_options(false, NULL);
	init(sSamplingRate, NCHANS, sBufferFrameCount, NULL, NULL, NULL);
}

//  The RTcmix constructor with settable SR, NCHANS; default RTBUFSAMPS
RTcmix::RTcmix(float tsr, int tnchans)
{
	init_options(false, NULL);
	init(tsr, tnchans, sBufferFrameCount, NULL, NULL, NULL);
}

//  The RTcmix constructor with settable SR, NCHANS, RTBUFSAMPS, and up to
//  3 "opt=value" options

RTcmix::RTcmix(float tsr, int tnchans, int bsize,
			   const char *opt1, const char *opt2, const char *opt3)
{
   init_options(false, NULL);
   init(tsr, tnchans, bsize, opt1, opt2, opt3);
}

// This is called by RTcmixMain constructor, which does all the work.

RTcmix::RTcmix(bool dummy) {}

// This is called when the entire system goes away.

RTcmix::~RTcmix()
{
    rtcmix_debug("~RTcmix", "shutting down and freeing memory");
	setRunStatus(RT_SHUTDOWN);
	waitForMainLoop();	// This calls close()
	free_globals();
#ifdef EMBEDDED
	destroy_parser();	// clean up symbols, etc
#endif
    rtcmix_debug("~RTcmix", "done");
}

//  The actual initialization method called by the imbedded constructors

void
RTcmix::init(float tsr, int tnchans, int bsize,
			 const char *opt1, const char *opt2, const char *opt3)
{
	// for rtsetparams -- these aren't 0-ed out
	// so no need to dimension them at MAXDISPARGS
	double p[3];

    rtcmix_debug("RTcmix::init", "entered");
#ifdef SGI
   flush_all_underflows_to_zero();
#endif

	// set up the command lists, etc.
	ug_intro();		/* introduce standard routines */
	// no profiles!  See the note above about DSOs
 
	setbuf(stdout, NULL);	/*  Want to see stdout errors */

	int nargs = 0;
	// set options if any are non-null
	p[0] = STRINGIFY(opt1);
	if (opt1) ++nargs;
	p[1] = STRINGIFY(opt2);
	if (opt2) ++nargs;
	p[2] = STRINGIFY(opt3);
	if (opt3) ++nargs;

	if (nargs)
		set_option(p, nargs);

	// set the sampling rate and nchannels
	p[0] = tsr;
	p[1] = tnchans;
	p[2] = bsize;
	nargs = 3;
	rtsetparams(p, nargs);

	if (RTOption::play() || RTOption::record()) {
		int retcode = runMainLoop();
		if (retcode != 0)
			fprintf(stderr, "runMainLoop() failed\n");
	}
	else {
		// If we are not playing or recording from HW, we cannot be interactive
        setInteractive(false);
	}
    rtcmix_debug("RTcmix::init", "exited");
}

double RTcmix::offset(double *p, int n_args)
{
	if (n_args < 1 || n_args > 2) {
		rtcmix_advise("rtoffset", "Usage: rtoffset(offset_time [, skip_preroll])");
		return 0;
	}
	bufTimeOffset = p[0];
	runToOffset = (n_args == 1) ? true : p[1] == 0.0;
	if (rtrecord) {
		rtcmix_advise("rtoffset", "Cannot skip forward when recording");
        bufTimeOffset = 0.0;
		runToOffset = false;
		return bufTimeOffset;
	}
	rtcmix_advise("rtoffset", "Starting playback at time %.3f %s preroll.", p[0], runToOffset ? "with" : "without");
	return bufTimeOffset;
}

// numeric p-field sending command.  The first "double" is to disambiguate
// from the string-sending command (below).  An old holdover from ancient
// cmix, but it's a handy thing
Instrument *
RTcmix::cmd(const char *name, int n_args, double p0, ...)
{
	va_list ap;
	int i;
	double p[MAXDISPARGS];
	void   *retval;

	p[0] = p0;
	va_start(ap, p0); // start variable list after p0
		for (i = 1; i < n_args; i++)
			p[i] = va_arg(ap, double);
	va_end(ap);

	(void) ::dispatch(name, p, n_args, &retval);

	return (Instrument *) retval;
}

// string p-field sending command.  the first "const char*" is to disambiguate
// from the double version above
Instrument *
RTcmix::cmd(const char *name, int n_args, const char* p0, ...)
{
	// these are not time-stamped as above... change if we need to!
	va_list ap;
	int i;
	char st[MAXDISPARGS][100];
	double p[MAXDISPARGS];
	void *retval;

	// this kludge dates from the olden days!
	strcpy(st[0], p0);
	p[0] = STRING_TO_DOUBLE(st[0]);
	va_start(ap, p0); // start variable list after p0
	for (i = 1; i < n_args; i++) {
		strncpy(st[i], va_arg(ap, char*), 100);
		p[i] = STRING_TO_DOUBLE(st[i]);
	}
	va_end(ap);

	(void) ::dispatch(name, p, n_args, &retval);

	return (Instrument *) retval;
}

#include <PFieldSet.h>
#include <PField.h>

// new PFieldSet sending command.
Instrument *
RTcmix::cmd(const char *name, const PFieldSet &pfSet)
{
	int nFields = pfSet.size();
	Arg	*arglist = new Arg[nFields];
	Arg retArg;
	
	// Copy PField pointers into arglist
	
	for (int field = 0; field < nFields; ++field) {
		arglist[field] = createPFieldHandle(&pfSet[field]);
	}

	try {
		(void) dispatch(name, arglist, nFields, &retArg);
	}
	catch (...) {
		delete [] arglist;
		throw;
	}

	delete [] arglist;
	
	// Extract and return instrument pointer.
	
	Handle rethandle = (Handle) retArg;
	if (rethandle->type == InstrumentPtrType)
		return (Instrument *) rethandle->ptr;
	else
		return NULL;
}

// for commands with no params -- the double return val is because
// that's what these commands generally do.
double
RTcmix::cmd(const char *name)
{
	// these are not time-stamped as above... change if we need to!
	double p[MAXDISPARGS]; // for passing into dispatch only
	double retval;

	retval = ::dispatch(name, p, 0, NULL);

	return(retval);
}

// This is s duplicate of the RTcmix::cmd() function, except that
// instead of returning an Inst*, it returns the double value
// of the RTcmix command that was invoked
double
RTcmix::cmdval(const char *name, int n_args, double p0, ...)
{
	va_list ap;
	int i;
	double p[MAXDISPARGS];
	void   *retval;

	p[0] = p0;
	va_start(ap, p0); // start variable list after p0
	for (i = 1; i < n_args; i++)
		p[i] = va_arg(ap, double);
	va_end(ap);

	return ::dispatch(name, p, n_args, &retval);
}

// This is s duplicate of the RTcmix::cmd() string-passing function, except
// that instead of returning an Inst*, it returns the double value
// of the RTcmix command that was invoked
double
RTcmix::cmdval(const char *name, int n_args, const char* p0, ...)
{
	// these are not time-stamped as above... change if we need to!
	va_list ap;
	int i;
	char st[MAXDISPARGS][100];
	double p[MAXDISPARGS];
	void *retval;

	// this kludge dates from the olden days!
	strcpy(st[0], p0);
	p[0] = STRING_TO_DOUBLE(st[0]);
	va_start(ap, p0); // start variable list after p0
	for (i = 1; i < n_args; i++) {
		strcpy(st[i], va_arg(ap, char*));
		p[i] = STRING_TO_DOUBLE(st[i]);
	}
	va_end(ap);

	return ::dispatch(name, p, n_args, &retval);
}


void RTcmix::printOn()
{
	RTOption::print(MMP_DEBUG);	// XXX REVERT ME!
	RTOption::reportClipping(true);

	/* Banner */
	char tbuf[128];
	snprintf(tbuf, 128, "--------> %s %s <--------\n", RTCMIX_NAME, RTCMIX_VERSION);
	rtcmix_advise(NULL, tbuf);
}

void RTcmix::printOff()
{
	RTOption::print(MMP_FATAL);
	RTOption::reportClipping(false);
}

void RTcmix::panic()
{
	run_status = RT_PANIC;	// resets itself in other thread
}

// This method is called by imbedded apps only, and only those apps which
// do not write to an audio device.  It causes the audio loop to run until
// it completes, and run() blocks until it does.

void RTcmix::run()
{
	int retcode;
	if (!RTOption::play() && !RTOption::record() && rtfileit == 1) {
		/* Create scheduling/audio thread. */
		rtcmix_debug(NULL, "RTcmix::run calling runMainLoop()");
		retcode = runMainLoop();
		if (retcode != 0) {
			rtcmix_warn(NULL, "RTcmix::run: runMainLoop() failed");
		}
		else {
			/* Wait for audio thread. */
			rtcmix_debug(NULL, "RTcmix::run calling waitForMainLoop()");
			retcode = waitForMainLoop();
			if (retcode != 0) {
				rtcmix_warn(NULL, "RTcmix::run: waitForMailLoop() failed");
			}
		}
	}
}

bool RTcmix::isInputAudioDevice(int fdIndex)
{
    return inputFileTable[fdIndex].isAudioDevice();
}

const char * RTcmix::getInputPath(int fdIndex)
{
    return inputFileTable[fdIndex].fileName();
}

int RTcmix::getBusCount()
{
	return busCount;
}

void RTcmix::setBufTimeOffset(float inOffset, bool inRunToOffset) {
	bufTimeOffset = inOffset; runToOffset = inRunToOffset;
}

void RTcmix::registerAudioStartCallback(AudioCallback callback, void *context)
{
    audioStartCallbacks.push_back(CallbackInfo(callback, context));
}

void RTcmix::unregisterAudioStartCallback(AudioCallback callback, void *context)
{
    for (std::vector<CallbackInfo>::iterator iter = audioStartCallbacks.begin();
         iter != audioStartCallbacks.end();
         ++iter) {
        if (iter->callback == callback && iter->context == context) {
            audioStartCallbacks.erase(iter);
            break;
        }
    }
}

void RTcmix::registerAudioStopCallback(AudioCallback callback, void *context)
{
    audioStopCallbacks.push_back(CallbackInfo(callback, context));
}

void RTcmix::unregisterAudioStopCallback(AudioCallback callback, void *context)
{
    for (std::vector<CallbackInfo>::iterator iter = audioStopCallbacks.begin();
         iter != audioStopCallbacks.end();
         ++iter) {
        if (iter->callback == callback && iter->context == context) {
            audioStopCallbacks.erase(iter);
            break;
        }
    }
}

void RTcmix::registerDestroyCallback(AudioCallback callback, void *context)
{
    destroyCallbacks.push_back(CallbackInfo(callback, context));
}

void RTcmix::callStartCallbacks()
{
    for (std::vector<CallbackInfo>::iterator iter = audioStartCallbacks.begin();
         iter != audioStartCallbacks.end();
         ++iter)
    {
        (iter->callback)(iter->context);
    }
}

void RTcmix::callStopCallbacks()
{
    for (std::vector<CallbackInfo>::iterator iter = audioStopCallbacks.begin();
         iter != audioStopCallbacks.end();
         ++iter)
    {
        (iter->callback)(iter->context);
    }
}

void RTcmix::callDestroyCallbacks()
{
    for (std::vector<CallbackInfo>::iterator iter = destroyCallbacks.begin();
         iter != destroyCallbacks.end();
         ++iter)
    {
        (iter->callback)(iter->context);
    }
}

int RTcmix::startAudio(AudioDeviceCallback renderCallback, AudioDeviceCallback doneCallback, void *inContext)
{
	rtcmix_debug(NULL, "RTcmix::startAudio entered");
	if (audioDevice && audioDevice->isOpen()) {
		// Set done callback on device.
		if (doneCallback != NULL)
			audioDevice->setStopCallback(doneCallback, inContext);
        // Notify any listeners we are starting
        callStartCallbacks();
		// Start audio output device, handing it our render callback.
		if (audioDevice->start(renderCallback, inContext) != 0) {
			rtcmix_warn(NULL, "Audio device failed to start: %s", audioDevice->getLastError());
			audioDevice->close();
			return -1;
		}
        rtcmix_debug(NULL, "RTcmix::startAudio finished");
		return 0;	// Playing, thru HW and/or to FILE.
	}
	rtcmix_warn(NULL, "Audio device was NULL or not open");
	return -1;
}

#ifdef EMBEDDEDAUDIO

#include "EmbeddedAudioDevice.h"

int RTcmix::runAudio(void *inAudioBuffer, void *outAudioBuffer, int frameCount)
{
	EmbeddedAudioDevice *device = (EmbeddedAudioDevice *) audioDevice;
	return (device && device->run(inAudioBuffer, outAudioBuffer, frameCount) == true) ? 0 : -1;
}

#endif

int RTcmix::stopAudio()
{
	rtcmix_debug(NULL, "RTcmix::stopAudio entered");
	return audioDevice->stop();
}

void RTcmix::close()
{
	rtcmix_debug(NULL, "RTcmix::close entered");
	setRunStatus(RT_SHUTDOWN);
	AudioDevice *dev = audioDevice;
	audioDevice = NULL;
	delete dev;
	audio_config = NO;
	free_buffers();
#ifdef MULTI_THREAD
	InputFile::destroyConversionBuffers();
#endif
	rtcmix_debug(NULL, "RTcmix::close exited");
}

int RTcmix::resetAudio(float, int, int, bool)
{
	rtcmix_debug(NULL, "RTcmix::resetAudio entered");
	run_status = RT_GOOD;
	return 0;
}

// Tempo functions.  These are now in RTcmix to allow interaction between them
// and possible MIDI output routines that would need to know.

double
RTcmix::tbase(double p[], int n_args)
{
    BASIS = p[0];
    return 0.0;
}

double
RTcmix::tempo(double p[], int n_args)
{
    short m;
    double dur,prvbt;
    if (!n_args) {
        rtcmix_advise("tempo", "Tempo changes cleared out");
        tempo_set = 0;
        return -1.0;
    }
    for(m=0;m<TLEN;m++) { xtime[m]=tempovals[m]=rxtime[m]=accel[m]=0.; }
    for(m=0, numTimePoints=1; m < TLEN; m += 2, numTimePoints++) {
        if ((m) && (m >= n_args)) break;
        xtime[numTimePoints] = p[m];
        tempovals[numTimePoints] = p[m + 1] / BASIS;
        if (tempovals[numTimePoints] == 0.0) { return die("tempo", "tempo value cannot be zero - did you reverse your arguments?"); }
    }
    tempo_set = 1;
    rxtime[numTimePoints] = xtime[numTimePoints] = .999999e+10;
    tempovals[numTimePoints] = tempovals[numTimePoints - 1];
    tempovals[0] = tempovals[1];
    xtime[0] = rxtime[0] = accel[0] = prvbt = 0.;
    for(m=0; m < numTimePoints; m++) {
        dur = xtime[m+1] - xtime[m];
        if (dur == 0.0f) {
            accel[m] = 0.0f;
            rxtime[m+1] = rxtime[m];
        }
        else 	{
            accel[m] = (pow(tempovals[m+1],2.0) - pow(tempovals[m],2.0))/(2.0*dur);
            if (accel[m] == 0.0) {
                rxtime[m+1] = dur/tempovals[m]+prvbt;
            }
            else {
                rxtime[m+1] = (sqrt(pow(tempovals[m],2.0)+ (2.0* accel[m]*dur))-tempovals[m]) / accel[m] + prvbt;
            }
        }
        prvbt = rxtime[m+1];
    }
/*
for(m=0; m<=numTimePoints; m++) printf("%d %f %f %f %f\n",m,tempovals[m],accel[m],rxtime[m],xtime[m]);
*/
    return 0.0;
}

double RTcmix::time_beat(double timein)
{
    int m = 0;
    double durp = 0;

    if (timein < 0.0f)
        timein = 0.0f;

    if (!tempo_set) return(timein);

    if (timein > 0.0f) {
        for(m=0; m <= numTimePoints; m++) {
            if (timein > xtime[m] && timein <= xtime[m+1]) {
                durp = timein-xtime[m];
                break;
            }
        }
    }
    if (accel[m] == 0.0f) {
        return(durp/tempovals[m] + rxtime[m]);
    }
    return (sqrt(pow(tempovals[m],2.0) + (2.0 * accel[m] * durp)) - tempovals[m]) / accel[m] + rxtime[m];
}


double RTcmix::beat_time(double beatin)    /* returns beats from times */
{
    int m=0;
    if (!tempo_set) return(beatin);

    if (beatin != 0.0f) {
        for(m=0; m <= numTimePoints; m++) {
            if (beatin > rxtime[m] && beatin <= rxtime[m+1]) {
                break;
            }
        }
    }
    if (accel[m] == 0.0f) {
        return((beatin-rxtime[m])*tempovals[m] + xtime[m]);
    }
    return (pow(((beatin-rxtime[m]) * accel[m]+tempovals[m]),2.0) - pow(tempovals[m],2.0)) / (2.0 * accel[m]) + xtime[m];
}

// This is from the RTsockfuncs.c file in ../lib.  It's such a handy
// function, I just had to have it here, too!  BGG
/* RTtimeit takes a floating point number of seconds (interval) and a pointer
   to a void-returning function and sets up a timer to call that function
   every interval seconds.  Setting interval to 0.0 should disable the
   timer */
// I notice that because this uses SIGALRM it will wake up sleep()'s
// put sleep()s in a while loop to keep the process alive...
void RTtimeit(float interval, sig_t func)
{
	struct timeval tv;
	struct itimerval itv;

	tv.tv_sec = (int)(interval);
	tv.tv_usec = (int)((interval - (float)tv.tv_sec) * 1000000.0);
	itv.it_interval = tv;
	itv.it_value = tv;
	setitimer(ITIMER_REAL, &itv, NULL);
	signal(SIGALRM, func);
}

