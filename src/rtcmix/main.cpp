/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
//#define DBUG
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
#include "../rtstuff/rt.h"
#include "sockdefs.h"
#include "notetags.h"           // contains defs for note-tagging
#include "../H/dbug.h"
#include "rtcmix_parse.h"


extern "C" {
  int ug_intro();
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
static void
init_globals()
{
   int i;

   RTBUFSAMPS = 8192;           /* default, modifiable with rtsetparams */
   NCHANS = 2;
   audioNCHANS = 0;

#ifdef LINUX
   for (i = 0; i < MAXBUS; i++)
      in_port[i] = out_port[i] = 0;
#endif /* LINUX */
#ifdef MACOSX
#endif
#ifdef SGI
   in_port = 0;
   out_port = 0;
#endif /* SGI */

   rtQueue = new RTQueue[MAXBUS*3];

   rtInteractive = 0;
   noParse = 0;
   socknew = 0;
   rtsetparams_called = 0;

   audio_on = 0;
   audio_config = 1;
   play_audio = 1;              /* modified with set_option */
   full_duplex = 0;
   check_peaks = 1;
   report_clipping = 1;

   /* I can't believe these were never initialized */
   baseTime = 0;
   elapsed = 0;
   schedtime = 0;

   output_data_format = -1;
   output_header_type = -1;
   normalize_output_floats = 0;
   is_float_format = 0;
   rtoutsfname = NULL;

   tags_on = 0;

   rtfileit = 0;                /* signal writing to soundfile */
   rtoutfile = 0;

   print_is_on = 1;

   for (i = 0; i < MAXBUS; i++) {
      AuxToAuxPlayList[i] = -1; /* The playback order for AUX buses */
      ToOutPlayList[i] = -1;    /* The playback order for AUX buses */
      ToAuxPlayList[i] =-1;     /* The playback order for AUX buses */
   }

   for (i = 0; i < MAX_INPUT_FDS; i++)
      inputFileTable[i].fd = NO_FD;

   init_buf_ptrs();
}


/* ------------------------------------------------------- sigint_handler --- */
static void
sigint_handler(int signo)
{
#ifdef DBUG
   printf("Signal handler called (signo %d)\n", signo);
#endif

   if (rtsetparams_called) {
      close_audio_ports();
      rtreportstats();
      rtcloseout();
   }
   else
      closesf();

   exit(1);
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

   /* Call sigint_handler on cntl-C. */
   if (signal(SIGINT, sigint_handler) == SIG_ERR) {
      fprintf(stderr, "Error installing signal handler.\n");
      exit(1);
   }

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
               report_clipping = 0; /* (then fall through) */
            case 'q':               /* quiet */
               print_is_on = 0;
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
   if (print_is_on)
      printf("--------> %s %s (%s) <--------\n",
                                      RTCMIX_NAME, RTCMIX_VERSION, argv[0]);

   ug_intro();                  /* introduce standard routines */
   profile();                   /* introduce user-written routines etc. */
   rtprofile();                 /* introduce real-time user-written routines */

   setbuf(stdout, NULL);        /*  Want to see stdout errors */

   /* In rtInteractive mode, we set up RTcmix to listen for score data
      over a socket, and then parse this, schedule instruments, and play
      them concurrently. The socket listening and parsing go in one
      thread, and the scheduler and instrument code go in another.

      When not in rtInteractive mode, RTcmix parses the score, schedules
      all instruments, and then plays them -- in that order. No threads.
   */
   if (rtInteractive) {

      if (print_is_on)
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
      fprintf(stdout, "creating inTraverse() thread\n");
#endif
      retcode = pthread_create(&inTraverseThread, NULL, inTraverse,
                                                             (void *) "");
      if (retcode != 0) {
         fprintf(stderr, "inTraverse() thread create failed\n");
      }

      /* Join parsing thread. */
#ifdef DBUG
      fprintf(stdout, "joining sockit() thread\n");
#endif
      retcode = pthread_join(sockitThread, NULL);
      if (retcode != 0) {
         fprintf(stderr, "sockit() thread join failed\n");
      }

      /* Join scheduling thread. */
#ifdef DBUG
      fprintf(stdout, "joining inTraverse() thread\n");
#endif
       retcode = pthread_join(inTraverseThread, NULL);
      if (retcode != 0) {
         fprintf(stderr, "inTraverse() thread join failed\n");
      }

      if (!noParse)
         destroy_parser();
   }
   else {
      int status;

      status = parse_score(xargc, xargv);
#ifdef PYTHON
      /* Have to reinstall this after running Python interpreter. (Why?) */
      if (signal(SIGINT, sigint_handler) == SIG_ERR) {
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
         inTraverse(NULL);
	  }
      else
         exit(status);

      destroy_parser();
   }

   /* DJT:  this instead of above joins */
   /* while (rtInteractive) {}; */

   closesf_noexit();

   return 0;
}


