// OK -- this is now RTcmix.C, hacked from the original "main.C"
// 	in standalone RTcmix.  The idea is to make this a compile-in
//	imbRTcmix.o lib that can be called from within other main()
//	progs.  -- BGG 11/2002

/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include "pRTcmix.h"

//#define DBUG
//#define DENORMAL_CHECK
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <iostream.h>
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
#include "rt.h"
#include "../rtstuff/heap/heap.h"
#include "maxdispargs.h"
#include "dbug.h"
extern "C" {
#include "rtcmix_parse.h"
}

extern "C" {
	int ug_intro();
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

	RTBUFSAMPS = 8192; //default, modifiable with rtsetparams
	NCHANS = 2;
	audioNCHANS = 0;
	SR = 44100.0; // what the heck...
	bufStartSamp = 0;

#ifdef LINUX
	for (i = 0; i < MAXBUS; i++) in_port[i] = out_port[i] = 0;
#endif /* LINUX */
#ifdef MACOSX
#endif
#ifdef SGI
	in_port = 0;
	out_port = 0;
#endif /* SGI */

	rtQueue = new RTQueue[MAXBUS*3];

	rtInteractive = 1; // keep the heap going for this object
	rtsetparams_called = 0; // will call at object instantiation, though

	audio_on = 0;
	audio_config = 1;
	play_audio = 1;              /* modified with set_option */
	full_duplex = 0;
	check_peaks = 1;
	report_clipping = 1;

	/* I can't believe these were never initialized */
	// hey, I'm that kinda guy! :-)
	baseTime = 0;
	elapsed = 0;
	schedtime = 0;

	output_data_format = -1;
	output_header_type = -1;
	normalize_output_floats = 0;
	is_float_format = 0;
	rtoutsfname = NULL;

	rtfileit = 0;                /* signal writing to soundfile */
	rtoutfile = 0;

	print_is_on = 0; // default is off for the RTcmix object

	for (i = 0; i < MAXBUS; i++) {
		AuxToAuxPlayList[i] = -1; /* The playback order for AUX buses */
		ToOutPlayList[i] = -1;    /* The playback order for AUX buses */
		ToAuxPlayList[i] =-1;     /* The playback order for AUX buses */
	}

	for (i = 0; i < MAX_INPUT_FDS; i++) inputFileTable[i].fd = NO_FD;

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
	init(SR, NCHANS, RTBUFSAMPS);
}

//  The RTcmix constructor with settable SR, NCHANS; default RTBUFSAMPS
RTcmix::RTcmix(float tsr, int tnchans)
{
	init_globals();
	init(tsr, tnchans, RTBUFSAMPS);
}

//  The RTcmix constructor with settable SR, NCHANS, and RTBUFSAMPS
RTcmix::RTcmix(float tsr, int tnchans, int bsize)
{
	init_globals();
	init(tsr, tnchans, bsize);
}

//  The actual initialization method called by the constructors

void
RTcmix::init(float tsr, int tnchans, int bsize)
{
	int retcode;		/* for mutexes */
	pthread_t inTraverseThread;

	// for rtsetparams -- I forget why it's set up with both double
	// and float p-field arrays.  Also, these aren't 0-ed out
	// so no need to dimension them at MAXDISPARGS
	float p[3];
	double pp[3];

#ifdef SGI
	flush_all_underflows_to_zero();
#endif

	// set up the command lists, etc.
	ug_intro();		/* introduce standard routines */
	// no profiles!  See the note above about DSOs

	setbuf(stdout, NULL);	/*  Want to see stdout errors */

	// set the sampling rate and nchannels
	p[0] = pp[0] = tsr;
	p[1] = pp[1] = tnchans;
	p[2] = pp[2] = bsize;
	rtsetparams(p, 3, pp);

	retcode = pthread_create(&inTraverseThread, NULL, inTraverse,
					(void *) "");
	if (retcode != 0)
		fprintf(stderr, "inTraverse() thread create failed\n");
}


// numeric p-field sending command.  The first "double" is to disambiguate
// from the string-sending command (below).  An old holdover from ancient
// cmix, but it's a handy thing
Instrument *RTcmix::cmd(char name[], int n_args, double p0, ...)
{
	double buftime,sec,usec;
	struct timeval tv;
	struct timezone tz;
	va_list ap;
	int i;
	double p[MAXDISPARGS];
	void   *retval;

	buftime = (double)RTBUFSAMPS/SR;

	gettimeofday(&tv, &tz);
	sec = (double)tv.tv_sec;
	usec = (double)tv.tv_usec;
	pthread_mutex_lock(&schedtime_lock);
	schedtime = (((sec * 1e6) + usec) - baseTime) * 1e-6;
	schedtime += ((double)elapsed/(double)SR);
	schedtime += buftime;
	pthread_mutex_unlock(&schedtime_lock);

	// schedtime is accessed in rtsetoutput() to set the current
	// time.  Plus, in interactive mode we have to run a slight delay
	// from "0" or we wind up scheduling events in the past.

	p[0] = p0;
	va_start(ap, p0); // start variable list after p0
		for (i = 1; i < n_args; i++)
			p[i] = va_arg(ap, double);
	va_end(ap);

	(double) parse_dispatch(name, p, n_args, &retval);

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

	(double) parse_dispatch(name, p, n_args, &retval);

	return (Instrument *) retval;
}

// for commands with no params -- the double return val is because
// that's what these commands generally do.
double RTcmix::cmd(char name[])
{
	// these are not time-stamped as above... change if we need to!
	double p[MAXDISPARGS]; // for passing into parse_dispatch only
	double retval;

	retval = parse_dispatch(name, p, 0, NULL);

	return(retval);
}

void RTcmix::printOn()
{
	print_is_on = 1;
	report_clipping = 1;

	/* Banner */
	if (print_is_on) printf("--------> %s %s <--------\n",
			RTCMIX_NAME, RTCMIX_VERSION);
}

void RTcmix::perlparse(char *inBuf)
{
	double buftime,sec,usec;
	struct timeval tv;
	struct timezone tz;

	buftime = (double)RTBUFSAMPS/SR;
	
	gettimeofday(&tv, &tz);
	sec = (double)tv.tv_sec;
	usec = (double)tv.tv_usec;
	pthread_mutex_lock(&schedtime_lock);
	schedtime = (((sec * 1e6) + usec) - baseTime) * 1e-6;
	schedtime += ((double)elapsed/(double)SR);
	schedtime += buftime*5;
	pthread_mutex_unlock(&schedtime_lock);
	perl_parse_buf(inBuf);
}


void RTcmix::printOff()
{
	print_is_on = 0;
	report_clipping = 0;
}

void RTcmix::close()
{
	closesf_noexit();
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
