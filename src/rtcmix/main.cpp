/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
//#define DBUG
#define MAIN
#include <pthread.h>
#include <ctype.h>
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
#include "defs.h"
#include "notetags.h"           // contains defs for note-tagging
#include "../H/dbug.h"

extern "C" {
   int ug_intro();
   void *yyparse();
   int profile();
   void rtprofile();
#ifdef SGI
   void flush_all_underflows_to_zero();
#endif
#ifdef LINUX
   void flush_fpe(int sig);
#endif
}

rt_item *rt_list;     /* can't put this in globals.h because of rt.h trouble */

int interactive;     /* used in y.tab.c - don't remove this! */
Tree program;

#define CMAX 16    /* allow up to 16 command line args to be passed to subs */
char *aargv[CMAX];
char name[NAMESIZE];
int aargc;

/* <yyin> is yacc's input file. If we leave it alone, stdin will be used. */
extern FILE *yyin;

/* --------------------------------------------------------- init_globals --- */
static void
init_globals()
{
   RTBUFSAMPS = 8192;           /* default, modifyable with rtsetparams */
   NCHANS = 2;
   audioNCHANS = 0;

#ifdef LINUX
   for (int i = 0; i < MAXBUS; i++)
      in_port[i] = out_port[i] = 0;
#endif /* LINUX */
#ifdef SGI
   in_port = 0;
   out_port = 0;
#endif /* SGI */

   interactive = 1;
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

   output_data_format = -1;
   output_header_type = -1;
   normalize_output_floats = 0;
   is_float_format = 0;
   rtoutsfname = NULL;

   tags_on = 0;

   rtfileit = 0;                /* signal writing to soundfile */
   rtoutfile = 0;

   print_is_on = 1;

   Bus_Configed = NO;
   for(i=0;i<MAXBUS;i++) {
	 AuxToAuxPlayList[i] = -1; /* The playback order for AUX buses */
	 ToOutPlayList[i] = -1; /* The playback order for AUX buses */
	 ToAuxPlayList[i] =-1; /* The playback order for AUX buses */
   }

   for (int i = 0; i < MAX_INPUT_FDS; i++)
      inputFileTable[i].fd = NO_FD;

   init_buf_ptrs();
}


/* ------------------------------------------------------- signal_handler --- */
static void
signal_handler(int signo)
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


/* ----------------------------------------------------------------- main --- */
int
main(int argc, char *argv[])
{
   int         i, j;
   int         retcode;                 /* for mutexes */
   char        *cp, *infile;
   pthread_t   sockitThread, inTraverseThread;

   init_globals();

#ifdef LINUX
   signal(SIGFPE, flush_fpe);          /* Install signal handler */
#endif
#ifdef SGI
   flush_all_underflows_to_zero();
#endif

   /* Call signal_handler on cntl-C. */
   if (signal(SIGINT, signal_handler) == SIG_ERR) {
      fprintf(stderr, "Error installing signal handler.\n");
      exit(1);
   }

   /* Copy command-line args, if any.
      Now, command-line args will be available to any subroutine
      in aargc; and char *aargv[], (both declared extern in ugens.h)
      parsing is exactly the same then as standard C routine.
      Maximum of CMAX arguments allowed.
   */
   for (j = 1; j < argc; j++)
      aargv[j - 1] = argv[j];
   aargc = argc;

   /* Banner */
   printf("--------> %s %s (%s) <--------\n",
                                      RTCMIX_NAME, RTCMIX_VERSION, argv[0]);

   /* Process command line. */
   if (argc >= 2) {
      while ((*++argv)[0] == '-') {
         argc -= 1;             /* Take away two args */
         for (cp = argv[0] + 1; *cp; cp++) {
            switch (*cp) {      /* Grap options */
               case 'i':               /* for separate parseit thread */
                  rtInteractive = 1;
                  audio_config = 0;
                  break;
               case 'n':               /* for separate parseit thread */
                  noParse = 1;
                  break;
               case 's':               /* set up a socket offset */
                  socknew = atoi(*++argv);
                  fprintf(stderr, "listening on socket: %d\n",
                                                         MYPORT + socknew);
                  argc -= 1;
                  break;
               case 'c':     /* set up for continuous control (note tags on) */
                  tags_on = 1;
                  printf("rtupdates enabled\n");
                  curtag = 1;          /* "0" is reserved for all notes */
                  for (i = 0; i < MAXPUPS; i++)     /* initialize element 0 */
                     pupdatevals[0][i] = NOPUPDATE;
                  break;
               case 'f':     /* use file name arg instead of stdin as score */
                  infile = *++argv;
                  yyin = fopen(infile, "r+");
                  if (yyin == NULL) {
                     printf("Can't open %s\n", infile);
                     exit(1);
                  }
                  fprintf(stderr, "Using file %s\n", infile);
                  argc -= 1;
                  break;
               default:
                  printf("Don't know about option '%c'\n", *cp);
            }
         }
         if (argc == 1)
            break;
      }
   }

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

      fprintf(stdout, "rtInteractive mode set\n");

      /* Read an initialization score. */
      if (!noParse) {
         cout << "Parsing once ...\n";
         yyparse();
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

      /* Join scheduling thread. */
#ifdef DBUG
      fprintf(stdout, "joining inTraverse() thread\n");
#endif
      retcode = pthread_join(inTraverseThread, NULL);
      if (retcode != 0) {
         fprintf(stderr, "inTraverse() thread join failed\n");
      }

      /* Join parsing thread. */
#ifdef DBUG
      fprintf(stdout, "joining sockit() thread\n");
#endif
      retcode = pthread_join(sockitThread, NULL);
      if (retcode != 0) {
         fprintf(stderr, "sockit() thread join failed\n");
      }

   }
   else {
      yyparse();
      inTraverse(NULL);
   }

   closesf();

   return 0;
}


