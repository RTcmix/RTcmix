// OK -- this is now RTcmix.C, hacked from the original "main.C"
// 	in standalone RTcmix.  The idea is to make this a compile-in
//	imbRTcmix.o lib that can be called from within other main()
//	progs.  -- BGG 11/2002

/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include "RTcmix.h"

//#define DBUG
//#define DENORMAL_CHECK
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <iostream.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#ifdef LINUX
   #include <sys/soundcard.h>
#endif
#ifdef SGI
   #include <dmedia/audio.h>
#endif

#include <globals.h>
#include <prototypes.h>
#include <ugens.h>
#include <version.h>
#include <Option.h>
#include <ug_intro.h>
#include "rt.h"
#include "heap.h"
#include "maxdispargs.h"
#include "dbug.h"

extern "C" {
	// I don't call the profiles here, because dead-time instruments
	// won't be compiled into the object file unless they are present at
	// the build (i.e. they aren't DSO's).  RT instruments have the
	// rtprofile() called when they get loaded.  Go Doug, go!
#ifdef SGI
  void flush_all_underflows_to_zero();
#endif
#ifdef LINUX
  void sigfpe_handler(int sig);
#endif
}

rt_item *rt_list;     /* can't put this in globals.h because of rt.h trouble */


/* --------------------------------------------------------- init_globals --- */
static void
init_globals()
{
   int i;

   Option::init();

   RTBUFSAMPS = (int) Option::bufferFrames();  /* modifiable with rtsetparams */
   NCHANS = 2;
   audioNCHANS = 0;
	SR = 44100.0; // what the heck...
	bufStartSamp = 0;

   rtQueue = new RTQueue[MAXBUS*3];

	rtInteractive = 1; // keep the heap going for this object
	rtsetparams_called = 0; // will call at object instantiation, though
   audio_config = 1;
   elapsed = 0;

#ifdef NETAUDIO
   netplay = 0;      // for remote sound network playing
#endif

   output_data_format = -1;
   output_header_type = -1;
   normalize_output_floats = 0;
   is_float_format = 0;
   rtoutsfname = NULL;

   rtfileit = 0;                /* signal writing to soundfile */
   rtoutfile = 0;

	Option::print(false);
	Option::reportClipping(false);

   for (i = 0; i < MAXBUS; i++) {
      AuxToAuxPlayList[i] = -1; /* The playback order for AUX buses */
      ToOutPlayList[i] = -1;    /* The playback order for AUX buses */
      ToAuxPlayList[i] =-1;     /* The playback order for AUX buses */
   }

   for (i = 0; i < MAX_INPUT_FDS; i++)
      inputFileTable[i].fd = NO_FD;

   init_buf_ptrs();
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
	init_globals();
	init(SR, NCHANS, RTBUFSAMPS, NULL, NULL, NULL);
}

//  The RTcmix constructor with settable SR, NCHANS; default RTBUFSAMPS
RTcmix::RTcmix(float tsr, int tnchans)
{
	init_globals();
	init(tsr, tnchans, RTBUFSAMPS, NULL, NULL, NULL);
}

//  The RTcmix constructor with settable SR, NCHANS, RTBUFSAMPS, and up to
//  3 "opt=value" options

RTcmix::RTcmix(float tsr, int tnchans, int bsize,
			   const char *opt1, const char *opt2, const char *opt3)
{
   init_globals();
	init(tsr, tnchans, bsize, opt1, opt2, opt3);
}

//  The actual initialization method called by the constructors

void
RTcmix::init(float tsr, int tnchans, int bsize,
			 const char *opt1, const char *opt2, const char *opt3)
{
	int retcode;		/* for mutexes */

	// for rtsetparams -- I forget why it's set up with both double
	// and float p-field arrays.  Also, these aren't 0-ed out
	// so no need to dimension them at MAXDISPARGS
	float p[3];
	double pp[3];

#ifdef SGI
   flush_all_underflows_to_zero();
#endif

	run_status = RT_GOOD;	// Make sure status is good.

	// set up the command lists, etc.
	ug_intro();		/* introduce standard routines */
	// no profiles!  See the note above about DSOs
 
	setbuf(stdout, NULL);	/*  Want to see stdout errors */

	int nargs = 0;
	// set options if any are non-null
	p[0] = pp[0] = (double)(int) opt1;
	if (opt1) ++nargs;
	p[1] = pp[1] =  (double)(int) opt2;
	if (opt2) ++nargs;
	p[2] = pp[2] = (double)(int) opt3;
	if (opt3) ++nargs;

	if (nargs)
		set_option(p, nargs, pp);

	// set the sampling rate and nchannels
	p[0] = pp[0] = tsr;
	p[1] = pp[1] = tnchans;
	p[2] = pp[2] = bsize;
	nargs = 3;
	rtsetparams(p, nargs, pp);

	retcode = runMainLoop();
	if (retcode != 0)
		fprintf(stderr, "runMainLoop() failed\n");
}


// numeric p-field sending command.  The first "double" is to disambiguate
// from the string-sending command (below).  An old holdover from ancient
// cmix, but it's a handy thing
Instrument *RTcmix::cmd(char name[], int n_args, double p0, ...)
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

	(void) dispatch(name, p, n_args, &retval);

	return (Instrument *) retval;
}

// string p-field sending command.  the first "char*" is to disambiguate
// from the double version above
Instrument *RTcmix::cmd(char name[], int n_args, char* p0, ...)
{
	// these are not time-stamped as above... change if we need to!
	va_list ap;
	int i;
	char st[MAXDISPARGS][100];
	int tmpint;
	double p[MAXDISPARGS];
	void *retval;

	// this kludge dates from the olden days!
	strcpy(st[0], p0);
	tmpint = (int)st[0];
	p[0] = (double)tmpint;
	va_start(ap, p0); // start variable list after p0
		for (i = 1; i < n_args; i++) {
			strcpy(st[i], va_arg(ap, char*));
			tmpint = (int)st[i];
			p[i] = (double)tmpint;
      }
	va_end(ap);

	(void) dispatch(name, p, n_args, &retval);

	return (Instrument *) retval;
}

#include <PFieldSet.h>
#include <PField.h>

// new PFieldSet sending command.
Instrument *RTcmix::cmd(char name[], const PFieldSet &pfSet)
{
	void   *retval;
	int nFields = pfSet.size();
	Arg	*arglist = new Arg[nFields];
	Arg retArg;
	
	// Copy PField pointers into arglist
	
	for (int field = 0; field < nFields; ++field) {
		Handle handle = (Handle) malloc(sizeof(struct _handle));
		handle->type = PFieldType;
		handle->ptr = (void *) &pfSet[field];
		arglist[field] = handle;
	}

	(void) dispatch(name, arglist, nFields, &retArg);

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
double RTcmix::cmd(char name[])
{
	// these are not time-stamped as above... change if we need to!
	double p[MAXDISPARGS]; // for passing into dispatch only
	double retval;

	retval = dispatch(name, p, 0, NULL);

	return(retval);
}

// This is s duplicate of the RTcmix::cmd() function, except that
// instead of returning an Inst*, it returns the double value
// of the RTcmix command that was invoked
double RTcmix::cmdval(char name[], int n_args, double p0, ...)
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

	return dispatch(name, p, n_args, &retval);
}

// This is s duplicate of the RTcmix::cmd() string-passing function, except
// that instead of returning an Inst*, it returns the double value
// of the RTcmix command that was invoked
double RTcmix::cmdval(char name[], int n_args, char* p0, ...)
{
	// these are not time-stamped as above... change if we need to!
	va_list ap;
	int i;
	char st[MAXDISPARGS][100];
	int tmpint;
	double p[MAXDISPARGS];
	void *retval;

	// this kludge dates from the olden days!
	strcpy(st[0], p0);
	tmpint = (int)st[0];
	p[0] = (double)tmpint;
	va_start(ap, p0); // start variable list after p0
		for (i = 1; i < n_args; i++) {
			strcpy(st[i], va_arg(ap, char*));
			tmpint = (int)st[i];
			p[i] = (double)tmpint;
   }
	va_end(ap);

	return dispatch(name, p, n_args, &retval);
}


void RTcmix::printOn()
{
	Option::print(true);
	Option::reportClipping(true);

	/* Banner */
	printf("--------> %s %s <--------\n", RTCMIX_NAME, RTCMIX_VERSION);
}

void RTcmix::printOff()
{
	Option::print(false);
	Option::reportClipping(false);
}

void RTcmix::panic()
{
	run_status = RT_PANIC;
	//	sleep(2);
	//run_status = RT_GOOD;
}

void RTcmix::close()
{
	run_status = RT_SHUTDOWN;
	destroy_audio_devices();
	rtcloseout();
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
