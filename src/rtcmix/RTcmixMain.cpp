/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include <signal.h>
#include <stdlib.h>
#include <iostream>
// BGGx ww
//#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "RTcmixMain.h"
#include <AudioDevice.h>
#include <Option.h>
#include "version.h"
#include <ugens.h>
#include <ug_intro.h>
#include "prototypes.h"
#include "../parser/rtcmix_parse.h"
// BGGx ww
//#include <sockdefs.h>
#include <limits.h>
#include "heap.h"

using namespace std;

#ifdef NETAUDIO
extern int setnetplay(char *, char *);    // in setnetplay.cpp
#endif

/* ---------------------------------------------------------------- usage --- */
static void
usage()
{
   printf("\n"
      "Usage:  CMIX [options] [arguments] < minc_script.sco\n"
      "\n"
      "   or, to use Perl instead of Minc:\n"
      "        PCMIX [options] [arguments] < perl_script.pl\n"
      "   or:\n"
      "        PCMIX [options] perl_script.pl [arguments]\n"
      "\n"
      "   or, to use Python:\n"
      "        PYCMIX [options] [arguments] < python_script.py\n"
      "\n"
      "        options:\n"
      "           -i       run in interactive mode\n"
      "           -n       no init script (interactive mode only)\n"
      "           -o NUM   socket offset (interactive mode only)\n"
#ifdef LINUX
      "           -p NUM   set process priority to NUM (as root only)\n"
#endif
      "           -D NAME  audio device name\n"
#ifdef NETAUDIO
      "           -k NUM   socket number (netplay)\n"
      "           -r NUM   remote host ip (or name for netplay)\n"
#endif
      "          NOTE: -s, -d, and -e are not yet implemented\n"
      "           -s NUM   start time (seconds)\n"
      "           -d NUM   duration (seconds)\n"
      "           -e NUM   end time (seconds)\n"
      "           -f NAME  read score from NAME instead of stdin\n"
      "                      (Minc and Python only)\n"
      "           --debug  enter parser debugger (Perl only)\n"
      "           -q       quiet -- suppress print to screen\n"
      "           -Q       really quiet -- not even clipping or peak stats\n"
      "           -v NUM   set verbosity (print level 0-5)\n"
      "           -h       this help blurb\n"
      "        Other options, and arguments, passed on to parser.\n\n");
   exit(1);
}

#ifdef DEBUG_MEMORY

#include <string.h>

void *operator new(size_t size) {
	size_t memsize = (size > 4) ? size : 4;
	void *ptr = malloc(memsize);
	memset(ptr, 0xaa, memsize);
	return ptr;
}

void operator delete(void *mem) {
	if (mem) {
		memset(mem, 0xdd, 4);
		free(mem);
	}
}

#endif


extern "C" {
  int profile();
  void rtprofile();
	// I don't call the profiles here, because dead-time instruments
	// won't be compiled into the object file unless they are present at
	// the build (i.e. they aren't DSO's).  RT instruments have the
	// rtprofile() called when they get loaded.  Go Doug, go!
}

// RTcmixMain is the derived RTcmix class called by main()

// Private static state

int 			RTcmixMain::xargc;
char *			RTcmixMain::xargv[MAXARGS + 1];
char **			RTcmixMain::xenv = NULL;
int				RTcmixMain::interrupt_handler_called = 0;
int				RTcmixMain::signal_handler_called = 0;

int				RTcmixMain::noParse         = 0;
int				RTcmixMain::parseOnly       = 0;
int				RTcmixMain::socknew			= 0;
#ifdef NETAUDIO
int				RTcmixMain::netplay 		= 0;	// for remote sound network playing
#endif


// ------------------------------------------------------------- makeDSOPath ---
// Assuming that our default shared lib dir is in the same directory as our
// bin dir, return the path name of the shared lib dir.  Return empty string
// if path to RTcmix executable <progPath> isn't deep enough, or if derived
// dir name doesn't exist.  Caller is responsible for deleting returned 
// string.  -JGG

#define PATH_DELIMITER  '/'
#define DSO_DIRNAME     "shlib"     // FIXME: move to site.conf?

char *RTcmixMain::makeDSOPath(const char *progPath)
{
// BGGx mm -- none of this in windows
/*
   char *dsoPath = new char [PATH_MAX + 1];
   strncpy(dsoPath, progPath, PATH_MAX);
   dsoPath[PATH_MAX] = 0;

   // back up two delimiters (e.g., "RTcmix/bin/CMIX" to "RTcmix/")
   char *p = strrchr(dsoPath, PATH_DELIMITER);
   if (p) {
      *p = 0;
      p = strrchr(dsoPath, PATH_DELIMITER);
      if (p) {
         p++;
         strcpy(p, DSO_DIRNAME);

         // use directory only if it exists
         struct stat statbuf;
         if (stat(dsoPath, &statbuf) == 0 && S_ISDIR(statbuf.st_mode))
            return dsoPath;
      }
   }

   // give up in case there weren't two delimiters or dir is invalid
   dsoPath[0] = 0;
   return dsoPath;
*/ // BGGx ww
return NULL;
}

#ifdef EMBEDDED
// BGG mm -- got rid of argc and argv for max/msp
RTcmixMain::RTcmixMain() : RTcmix(false)
#else
RTcmixMain::RTcmixMain(int argc, char **argv, char **env) : RTcmix(false)
#endif
{
#ifndef EMBEDDED
   set_sig_handlers();
#endif

// FIXME: should consult a makefile variable to tell us whether we should
// let dsoPath constructed at run time override SHAREDLIBDIR.

// FIXME: But ... there is no reasonable way to get the app path at
// runtime anyway...

   // From comp.lang.c FAQ list - Question 19.31
   // Q: How can my program discover the complete pathname to the executable
   //    from which it was invoked?
   // A: argv[0] may contain all or part of the pathname, or it may contain
   //    nothing. You may be able to duplicate the command language
   //    interpreter's search path logic to locate the executable if the name
   //    in argv[0] is present but incomplete.  However, there is no
   //    guaranteed solution.

#ifdef EMBEDDED
	init_options(true, NULL);		// 'true' indicates we were called from main
	for (int i = 1; i <= MAXARGS; i++) xargv[i] = NULL;
	xargc = 1;
#else
   char *dsoPath = makeDSOPath(argv[0]);
   init_options(true, dsoPath);		// 'true' indicates we were called from main
   delete [] dsoPath;

   parseArguments(argc, argv, env);
#endif

   // Note:  What follows was done in main().  Some of it is identical
   // to RTcmix::init() for imbedded.  Factor this out.
#ifdef EMBEDDED
	RTPrintf("--------> %s %s <--------\n",
			RTCMIX_NAME, RTCMIX_VERSION);
#else
	if (Option::print())
	   RTPrintf("--------> %s %s (%s) <--------\n",
             RTCMIX_NAME, RTCMIX_VERSION, argv[0]);
#endif

  ::ug_intro();                /* introduce standard routines */
  ::profile();                 /* introduce user-written routines etc. */
  ::rtprofile();               /* introduce real-time user-written routines */

 //  setbuf(stdout, NULL);        /*  Want to see stdout errors */
}


void
RTcmixMain::run()
{
   pthread_t   sockitThread;
   int retcode;
   if (interactive()) {
		rtcmix_advise(NULL, "rtInteractive mode set\n");

#ifndef EMBEDDED
      if (!noParse) {
         int status;
         rtcmix_debug(NULL, "Parsing once ...\n");
         status = ::parse_score(xargc, xargv, xenv);
         if (status != 0)
            exit(1);
      }
#endif // EMBEDDED

// BGGx ww -- no sockit in windows
/*
      rtcmix_debug(NULL, "creating sockit() thread\n");
      retcode = pthread_create(&sockitThread, NULL, &RTcmixMain::sockit, (void *) this);
      if (retcode != 0) {
         rterror(NULL, "sockit() thread create failed\n");
      }
*/ // BGGx ww

      rtcmix_debug(NULL, "calling runMainLoop()\n");
      retcode = runMainLoop();
      if (retcode != 0) {
         rterror(NULL, "runMainLoop() failed\n");
      }

// BGGx ww -- no sockit in windows
/*
      rtcmix_debug(NULL, "joining sockit() thread\n");
      retcode = pthread_join(sockitThread, NULL);
      if (retcode != 0) {
         rterror(NULL, "sockit() thread join failed\n");
      }
*/ // BGGx ww

      rtcmix_debug(NULL, "calling waitForMainLoop()\n");
	  retcode = waitForMainLoop();
      if (retcode != 0) {
         rterror(NULL, "waitForMailLoop() failed\n");
      }

#ifndef EMBEDDED
     if (!noParse)
         destroy_parser();
#endif
   }
   else {
#ifdef EMBEDDED
		int status = 0;
#else
      int status = ::parse_score(xargc, xargv, xenv);
	   if (parseOnly) {
		   rtcmix_debug(NULL, "RTcmixMain::run: parse-only returned status %d", status);
		   exit(status);
	   }
#endif
#ifdef PYTHON
	  set_sig_handlers();
#endif
      if (status == 0) {
#ifdef LINUX
//		 if (priority != 0)
//			 if (setpriority(PRIO_PROCESS, 0, priority) != 0)
//			 	perror("setpriority");
#endif
		 rtcmix_debug(NULL, "RTcmixMain::run: calling runMainLoop()");
		  if ((status = runMainLoop()) == 0) {
			 rtcmix_debug(NULL, "RTcmixMain::run: calling waitForMainLoop()");
			 waitForMainLoop();
		  }
	  }
      else
         exit(status);

#ifndef EMBEDDED
      destroy_parser();		// DAS TODO: make this work?
#endif
   }

#ifndef EMBEDDED
   ::closesf_noexit();
#endif
}

/* ---------------------------------------------------- interrupt_handler --- */
void
RTcmixMain::interrupt_handler(int signo)
{
	// Dont do handler work more than once
	if (!interrupt_handler_called) {
		interrupt_handler_called = 1;
	   fprintf(stderr, "\n<<< Caught interrupt signal >>>\n");
       if (audioDevice) {
           fprintf(stderr, "flushing audio...\n");
       }
       // Notify rendering loop no matter what.
       run_status = RT_SHUTDOWN;

	   // Notify rendering loop.
	   run_status = RT_SHUTDOWN;
	   if (audioDevice) {
	       audioDevice->close();
	   }
	   if (!audioLoopStarted) {
		   closesf();	// We exit if we have not yet configured audio.
	   }
	}
}

/* ------------------------------------------------------- signal_handler --- */
void
RTcmixMain::signal_handler(int signo)
{
	// Dont do handler work more than once
	if (!signal_handler_called) {
		signal_handler_called = 1;
	   fprintf(stderr, "\n<<< Caught internal signal (%d) >>>\n", signo);

	   run_status = RT_ERROR;
	   switch (signo) {
	   default:
		   fflush(stdout);
		   fflush(stderr);
  	 	   exit(1);
	       break;
	   }
	}
}

void
RTcmixMain::set_sig_handlers()
{
   /* Call interrupt_handler on cntl-C. */
   if (signal(SIGINT, interrupt_handler) == SIG_ERR) {
      fprintf(stderr, "Error installing signal handler.\n");
      exit(1);
   }
   /* Call signal_handler on segv, etc. */
   if (signal(SIGSEGV, signal_handler) == SIG_ERR) {
      fprintf(stderr, "Error installing signal handler.\n");
      exit(1);
   }
#if defined(SIGBUS)
   if (signal(SIGBUS, signal_handler) == SIG_ERR) {
      fprintf(stderr, "Error installing signal handler.\n");
      exit(1);
   }
#endif
}


#ifdef EMBEDDED
// BGG -- called by RTcmix_flushScore() in main.cpp (for the [flush] message)
void
RTcmixMain::resetQueueHeap()
{
	rtcmix_advise(NULL, "Flushing all instrument queues");
	run_status = RT_FLUSH;	// This gets reset in inTraverse()
}


// BGG mm -- this is the loading code for instrument development using rtcmix~
// this was mainly copied from loader.c (disabled in rtcmix~) and is called
// from the loadinst() function at the end of main.cpp

typedef void (*mm_loadFun)();

int 
RTcmixMain::doload(char *dsoPath)
{
// BGGx ww -- no loading under windows
/*
    int profileLoaded;
    mm_loadFun mm_loadrtprof = NULL;

	// unload the dso if already present (this method checks for that)
	theDSO.unload();

    if (theDSO.load(dsoPath) != 0) {
		rtcmix_warn("load", "Unable to dynamically load '%s': %s",
			 dsoPath, theDSO.error());
		return 0;
    }

    profileLoaded = 0;

	// BGG mm -- loading into rtcmix~ uses an auxiliary function in the
	// instrument definition called "mm_load_rtprofile()", which then
	// calls the rtprofile() [renamed mm_rtprofile()].
	if (theDSO.loadFunction(&mm_loadrtprof, "mm_load_rtprofile") == 0) {
      profileLoaded = 1; 
      (*mm_loadrtprof)(); 
#ifdef DBUG
      printf("Loaded RT profile\n"); 
#endif
     } 

    if (!profileLoaded) {
		rtcmix_warn("load", "Unable to find a profile routine in DSO '%s'", dsoPath);
		theDSO.unload();
		return 0;
    }

#ifndef EMBEDDED
// BGG -- this totally cause the maxmsp compile to stop
	rtcmix_advise("loader", "Loaded %s functions from shared library:\n\t'%s'.\n", (profileLoaded == 3) ? "standard and RT" :
		(profileLoaded == 2) ? "RT" : "standard", dsoPath);
#endif
*/ // BGGx ww

	return 1;
}

void
RTcmixMain::unload()
{
// BGGx ww
//	theDSO.unload();
}
#endif // EMBEDDED

