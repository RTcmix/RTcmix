/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#define MAIN
#include <globals.h>
#include <pthread.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream.h>
extern "C" {
#include "../H/ugens.h"
}

#include "../rtstuff/rt.h"

#include "sockdefs.h"  // Also includes defs.h
#include "notetags.h" // contains defs for note-tagging


#include "../H/dbug.h"

#ifdef SGI
	#include <dmedia/audio.h>
#endif
#ifdef LINUX
	#include <sys/soundcard.h>
	#include <signal.h>
#endif

extern "C" {
	int ug_intro();
	void *yyparse();
	int profile();
	void *sockit();
	Tree exct(Tree);
	int closesf();
#ifdef SGI
	void flush_all_underflows_to_zero();
#endif
#ifdef LINUX
	void flush_fpe (int sig);
#endif
}

rt_item *rt_list;   /* can't put this in globals.h because of rt.h trouble */

extern "C" void rtprofile();
extern "C" void *inTraverse();	
extern "C" void *yTraverse();
extern "C" void *traverse();
extern "C" void *rtsendsamps();

int interactive;    /* used in y.tab.c - don't remove this! */
Tree     program;            
#define CMAX 16     /* allow up to 16 command line args to be passed to subs */
int aargc;
char *aargv[CMAX];  /* this are declared in ugens.h */
char name[NAMESIZE];/* to store file name found by card scanner, accessed by
                       m_open in sound.c */
FILE *fp;


// *************************************************************************** 

static void
init_globals()
{
	// Set up buffer sizes 
	// These can be changed with rtsetparams
	
	RTBUFSAMPS = 8192; // default, modifyable with rtsetparams
// FIXME: NCHANS hasn't even been inited yet!
	MAXBUF = NCHANS * RTBUFSAMPS;
#ifdef DBUG	
	printf("init_globals():  MAXBUF = %d\n", MAXBUF);
#endif

	in_port = 0;
	out_port = 0;

	interactive = 1;
	rtInteractive = 0;	/* we set this with command-line option -r now */
	oldSched = 0;
	noParse = 0;
	socknew = 0;

	audio_on = 0;
	audio_config = 1;
	play_audio = 1;      /* modified with set_option */
	full_duplex = 0;

	tags_on = 0;

   rtfileit = 0;        /* signal writing to soundfile */
	rtoutfile = 0;
	rtoutswap = 0;

	print_is_on = 1;

   for (int i = 0; i < MAX_INPUT_FDS; i++)
      inputFileTable[i].fd = NO_FD;

	init_buf_ptrs();
}


main(int argc, char *argv[])
{
	int	i,j;
	char    *cp,*infile;
	int retcode;  // for mutexes

	init_globals();

	/* threads */
	pthread_t sockitThread, inTraverseThread;

#ifdef SGI
	flush_all_underflows_to_zero();
#endif
#ifdef LINUX
	/* Install signal handler */
	signal(SIGFPE, flush_fpe);
#endif

	/* copy command-line args, if any*/
	for(j = 1; j < argc; j++)
		aargv[j - 1] = argv[j];
	aargc = argc;
	/* Now, command-line args will be available to any subroutine
	   in aargc; and char *aargv[], (both declared extern in ugens.h)
	   parsing is exactly the same then as standard C routine.
	   Maximum of CMAX arguments allowed. */

	fprintf(stderr," ---------> RTcmix/%s <----------\n",argv[0]);

	/*
	 * "Introduce" functions and stuff to program
	 */
	fp = NULL;

	if(argc >= 2) {
	while((*++argv)[0] == '-') {
		argc -= 1; /* Take away two args */
		for(cp = argv[0]+1; *cp; cp++) {
			switch(*cp) { /* Grap options */
			case 'i':  /* This for separate parseit thread */
			  rtInteractive = 1;
			  audio_config = 0;
			  break;
			case 'n':  /* This for separate parseit thread */
			  noParse = 1;
			  break;
			case 'o':  /* Use the old scheduler */
			  oldSched = 1;
			  break;
                        case 's': // set up a socket offset
			  socknew = atoi(*++argv);
			  fprintf(stderr,"listening on socket: %d\n",MYPORT+socknew);
			  argc -= 1;
			  break;
			case 'c': // set up for continuous control (note tags on)
				tags_on = 1;
				printf("rtupdates enabled\n");
				curtag = 1; // "0" is reserved for all notes
				for (i = 0; i < MAXPUPS; i++)  // initialize element 0
					pupdatevals[0][i] = NOPUPDATE;
				break;
			case 'f':
			  infile = *++argv;
			  fp = fopen(infile,"r+");
			  if(fp == NULL) {
			    printf("Can't open %s\n",infile);
			    exit(-1);
			  }
			  fprintf(stderr,"Using file %s\n",infile);
			  break;
			default:  
				printf("don't know about option %c\n",*cp);
			}
		}
	        if(argc == 1) break;
		}
	}

	ug_intro();	/* introduce standard routines */
	profile();	/* introduce user-written routines etc. */
        rtprofile();	/* introduce real-time user-written routines */

	setbuf(stdout, NULL);	/*  Want to see stdout errors	*/

        if (rtInteractive) {
	  fprintf(stdout,"rtInteractive mode set\n");
	}

	/* Temporary ... just to get inits! */
        if (rtInteractive) {
	  /* Create the threads *************************/
	  /**********************************************/

	  /* Parsing threads ****************************/

	  if (!noParse) {
	    cout << "Parsing once ...\n";
	    yyparse();  /* a hack to read initialization scorefile */
	  }
	  fprintf(stdout,"creating sockit() thread\n");
	  retcode = pthread_create(&sockitThread, NULL, sockit, "");
	  if (retcode != 0) {
	    fprintf(stderr,"sockit() thread create failed\n");
	  }

	  /* Scheduling threads *************************/

	  fprintf(stdout,"creating inTraverse() thread\n");
	  retcode = pthread_create(&inTraverseThread, NULL, inTraverse, "");
	  if (retcode != 0) {
	    fprintf(stderr,"inTraverse() thread create failed\n");
	  }

	  /* Join the threads ****************************/
	  /***********************************************/

	  /* Scheduling threads ****************************/

	  fprintf(stdout,"joining inTraverse() thread\n");
	  retcode = pthread_join (inTraverseThread, NULL);
	  if (retcode != 0) {
	    fprintf(stderr,"inTraverse() thread join failed\n");
	  }

	  /* Parsing threads ****************************/

	  fprintf(stdout,"joining sockit() thread\n");
	  retcode = pthread_join (sockitThread, NULL);
	  if (retcode != 0) {
	    fprintf(stderr,"sockit() thread join failed\n");
	  }

	}
	else if (oldSched) {
	  cout << "Using old style parser\n";
	  yyparse();
	  traverse();  /* play instruments on the heap */
	}
	else {
	  yyparse();
	  inTraverse();
	}

#ifdef SGI
	if (out_port) while (ALgetfilled(out_port) > 0) {};
#endif
	closesf();
}

