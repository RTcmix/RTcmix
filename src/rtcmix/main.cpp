/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#define DBUG
//#define DENORMAL_CHECK
#define MAIN
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
#include <iostream.h>
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
#include "heap.h"
#include "sockdefs.h"
#include "notetags.h"           // contains defs for note-tagging
#include "dbug.h"
#include "../parser/rtcmix_parse.h"


extern "C" {
  int profile();
  void rtprofile();
#ifdef SGI
  void flush_all_underflows_to_zero();
#endif
#ifdef LINUX
  void sigfpe_handler(int sig);
#endif
}

rt_item *rt_list;     /* can't put this in globals.h because of rt.h trouble */


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
      "           -c       enable continuous control (rtupdates)\n"
#ifdef LINUX
      "           -p NUM   set process priority to NUM (as root only)\n"
#endif
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
      "           -Q       really quiet -- not even peak stats\n"
      "           -h       this help blurb\n"
      "        Other options, and arguments, passed on to parser.\n\n");
   exit(1);
}


/* --------------------------------------------------------- init_globals --- */
#include "Option.h"

static void
init_globals()
{
   int i;

   Option::init();
   Option::readConfigFile(Option::rcName());

   RTBUFSAMPS = (int) Option::bufferFrames();  /* modifiable with rtsetparams */
   NCHANS = 2;
   audioNCHANS = 0;

   rtQueue = new RTQueue[MAXBUS*3];

   rtInteractive = 0;
   noParse = 0;
   socknew = 0;
   rtsetparams_called = 0;
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

#ifdef RTUPDATE
   tags_on = 0;
#endif

   rtfileit = 0;                /* signal writing to soundfile */
   rtoutfile = 0;

   for (i = 0; i < MAXBUS; i++) {
      AuxToAuxPlayList[i] = -1; /* The playback order for AUX buses */
      ToOutPlayList[i] = -1;    /* The playback order for AUX buses */
      ToAuxPlayList[i] =-1;     /* The playback order for AUX buses */
   }

   for (i = 0; i < MAX_INPUT_FDS; i++)
      inputFileTable[i].fd = NO_FD;

   init_buf_ptrs();
}

static int interrupt_handler_called = 0;

/* ------------------------------------------------------- interrupt_handler --- */
static void
interrupt_handler(int signo)
{
	// Dont do handler work more than once
	if (!interrupt_handler_called) {
		interrupt_handler_called = 1;
	   fprintf(stderr, "\n<<< Caught interrupt signal >>>\n");

	   if (rtsetparams_called) {
		   stop_audio_devices();
	   }
	   else
		   closesf_noexit();
	}
}

static int signal_handler_called = 0;

/* ------------------------------------------------------- signal_handler --- */
static void
signal_handler(int signo)
{
	// Dont do handler work more than once
	if (!signal_handler_called) {
		signal_handler_called = 1;
	   fprintf(stderr, "\n<<< Caught internal signal (%d) >>>\n", signo);

	   switch (signo) {
	   default:
		   fflush(stdout);
		   fflush(stderr);
  	 	   exit(1);
	       break;
	   }
	}
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


/* ----------------------------------------------------------------- main --- */
int
main(int argc, char *argv[])
{
   int         i, j, k, l, xargc;
   int         retcode;                 /* for mutexes */
#ifdef LINUX
   int		   priority = 0;
#endif
   char        *infile;
   char        *xargv[MAXARGS + 1];
   pthread_t   sockitThread, inTraverseThread;
#ifdef NETAUDIO
   char        rhostname[60], thesocket[8];

   rhostname[0] = thesocket[0] = '\0';
#endif

   init_globals();

#ifdef LINUX
 #ifdef DENORMAL_CHECK
   detect_denormals();
 #endif
   signal(SIGFPE, sigfpe_handler);          /* Install signal handler */
#endif /* LINUX */
#ifdef SGI
   flush_all_underflows_to_zero();
#endif

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

   xargv[0] = argv[0];
   for (i = 1; i <= MAXARGS; i++)
      xargv[i] = NULL;
   xargc = 1;

   /* Process command line, copying any args we don't handle into
      <xargv> for parsers to deal with.
   */
   for (i = 1; i < argc; i++) {
      char *arg = argv[i];

      if (arg[0] == '-') {
         switch (arg[1]) {
            case 'h':
               usage();
               break;
            case 'i':               /* for separate parseit thread */
               rtInteractive = 1;
               audio_config = 0;
               break;
            case 'n':               /* for use in rtInteractive mode only */
               noParse = 1;
               break;
            case 'Q':               /* reall quiet */
               Option::reportClipping(false); /* (then fall through) */
            case 'q':               /* quiet */
               Option::print(false);
               break;
#ifdef LINUX
			case 'p':
               if (++i >= argc) {
                  fprintf(stderr, "You didn't give a priority number.\n");
                  exit(1);
               }
			   priority = atoi(argv[i]);
			   break;
#endif
#ifdef NETAUDIO
            case 'r':               /* set up for network playing */
              	if (++i >= argc) {
                  fprintf(stderr, "You didn't give a remote host ip.\n");
                  exit(1);
              	}
               /* host ip num */
               strcat(rhostname, "net:");
               strncat(rhostname, argv[i], 59-4);    /* safe strcat */
               rhostname[59] = '\0';
               netplay = 1;
               break;
            case 'k':               /* socket number for network playing */
                                    /* defaults to 9999 */
               if (++i >= argc) {
                  fprintf(stderr, "You didn't give a socket number.\n");
                  exit(1);
               }
               strncpy(thesocket, argv[i], 7);
               thesocket[7] = '\0';
               netplay = 1;
               break;
#endif
            case 'o':               /* NOTE NOTE NOTE: will soon replace -s */
            case 's':               /* set up a socket offset */
               if (++i >= argc) {
                  fprintf(stderr, "You didn't give a socket offset.\n");
                  exit(1);
               }
               socknew = atoi(argv[i]);
               printf("listening on socket: %d\n", MYPORT + socknew);
               break;
//            case 's':               /* start time (unimplemented) */
            case 'd':               /* duration to play for (unimplemented) */
            case 'e':               /* time to stop playing (unimplemented) */
               fprintf(stderr, "-s, -d, -e options not yet implemented\n");
               exit(1);
               break;
#ifdef RTUPDATE
            case 'c':     /* set up for continuous control (note tags on) */
               tags_on = 1;
               printf("rtupdates enabled\n");
               curtag = 1;          /* "0" is reserved for all notes */
			   curinst = 0;
			   curgen = 1;

               for (j = 0; j < MAXPUPS; j++)     /* initialize element 0 */
                  pupdatevals[0][j] = NOPUPDATE;
			   for(j = 0; j < MAXNUMTAGS; j++)
			   {
				   for(k = 0; k < MAXNUMPARAMS; k++)
				   {
					   numcalls[j][k] = 0;
					   cum_parray_size[j][k] = 0;
					   for(l = 0; l < MAXNUMCALLS; l++)
					   {
						   parray_size[j][k][l] = 0; //initilizes size of 
					   }						  // pfpath array
 
				   }
			   }
			   for(j = 0; j < MAXNUMINSTS; j++)
			   {
				   pi_goto[j] = -1;
				   for(k = 0; k < MAXNUMPARAMS; k++)
				   {
						numinstcalls[j][k] = 0;
						cum_piarray_size[j][k] = 0;
						for(l = 0; l < MAXNUMCALLS; l++)
						{
							piarray_size[j][k][l] = 0;
						}
				   }
			   }
               break;
#endif /* RTUPDATE */
            case 'f':     /* use file name arg instead of stdin as score */
               if (++i >= argc) {
                  fprintf(stderr, "You didn't give a file name.\n");
                  exit(1);
               }
               infile = argv[i];
               use_script_file(infile);
               break;
            case '-':           /* accept "--debug" and pass to Perl as "-d" */
               if (strncmp(&arg[2], "debug", 10) == 0)
                  xargv[xargc++] = strdup("-d");
               break;
            default:
               xargv[xargc++] = arg;    /* copy for parser */
         }
      }
      else
         xargv[xargc++] = arg;          /* copy for parser */

      if (xargc >= MAXARGS) {
         fprintf(stderr, "Too many command-line options.\n");
         exit(1);
      }
   }

   /* Banner */
   if (Option::print())
      printf("--------> %s %s (%s) <--------\n",
                                      RTCMIX_NAME, RTCMIX_VERSION, argv[0]);

#ifdef NETAUDIO
   if (netplay) {             /* set up socket for sending audio */
      extern int setnetplay(char *, char *);    // in setnetplay.C
      int status = setnetplay(rhostname, thesocket);
      if (status == -1) {
         fprintf(stderr, "Cannot establish network connection to '%s' for "
                                             "remote playing\n", rhostname);
         exit(-1);
      }
      fprintf(stderr, "Network sound playing enabled on machine '%s'\n",
                                                                  rhostname);
    }
#endif

   ug_intro();                  /* introduce standard routines */
   profile();                   /* introduce user-written routines etc. */
   rtprofile();                 /* introduce real-time user-written routines */

   setbuf(stdout, NULL);        /*  Want to see stdout errors */

   /* In rtInteractive mode, we set up RTcmix to listen for score data
      over a socket, and then parse this, schedule instruments, and play
      them concurrently. The socket listening and parsing go in one
      thread, and the scheduler and instrument code go in another.

      When not in rtInteractive mode, RTcmix parses the score, schedules
      all instruments, and then plays them -- in that order.
   */
   if (rtInteractive) {

      if (Option::print())
         fprintf(stdout, "rtInteractive mode set\n");

      /* Read an initialization score. */
      if (!noParse) {
         int status;

#ifdef DBUG
         cout << "Parsing once ...\n";
#endif
         status = parse_score(xargc, xargv);
         if (status != 0)
            exit(1);
      }

      /* Create parsing thread. */
#ifdef DBUG
      fprintf(stdout, "creating sockit() thread\n");
#endif
      retcode = pthread_create(&sockitThread, NULL, sockit, (void *) "");
      if (retcode != 0) {
         fprintf(stderr, "sockit() thread create failed\n");
      }

      /* Create scheduling thread. */
#ifdef DBUG
      fprintf(stdout, "calling runMainLoop()\n");
#endif
      retcode = runMainLoop();
      if (retcode != 0) {
         fprintf(stderr, "runMainLoop() failed\n");
      }

      /* Join parsing thread. */
#ifdef DBUG
      fprintf(stdout, "joining sockit() thread\n");
#endif
      retcode = pthread_join(sockitThread, NULL);
      if (retcode != 0) {
         fprintf(stderr, "sockit() thread join failed\n");
      }

      /* Wait for audio thread. */
#ifdef DBUG
      fprintf(stdout, "calling waitForMainLoop()\n");
#endif
	  retcode = waitForMainLoop();
      if (retcode != 0) {
         fprintf(stderr, "waitForMailLoop() failed\n");
      }

      if (!noParse)
         destroy_parser();
   }
   else {
      int status;

      status = parse_score(xargc, xargv);
#ifdef PYTHON
      /* Have to reinstall this after running Python interpreter. (Why?) */
      if (signal(SIGINT, signal_handler) == SIG_ERR) {
         fprintf(stderr, "Error installing signal handler.\n");
         exit(1);
      }
	   /* Call signal_handler on segv. */
	   if (signal(SIGSEGV, signal_handler) == SIG_ERR) {
		  fprintf(stderr, "Error installing signal handler.\n");
		  exit(1);
	   }
#endif
      if (status == 0) {
#ifdef LINUX
		 if (priority != 0)
			 if (setpriority(PRIO_PROCESS, 0, priority) != 0)
			 	perror("setpriority");
#endif
         if ((status = runMainLoop()) == 0)
			 waitForMainLoop();
	  }
      else
         exit(status);

      destroy_parser();
   }

   closesf_noexit();

   return 0;
}


