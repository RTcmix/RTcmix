/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <pthread.h>
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
//#include <sys/time.h>

#include <globals.h>
#include <prototypes.h>
#include "../rtstuff/rtdefs.h"
#include "sockdefs.h"
#include "../H/dbug.h"

#include "notetags.h"

// #define DBUG

#ifdef PFIELD_CLASS
   #define parse_dispatch old_parse_dispatch
#endif

extern "C" {
  void *sockit(void*)
  {

//     double buftime,sec,usec;
//     struct timeval tv;
//     struct timezone tz;
    char ttext[MAXTEXTARGS][512];
    int i,tmpint;

    // socket stuff
    int s, ns;
#ifdef LINUX
    unsigned int len;
#else
    int len;
#endif
    struct sockaddr_in sss;
    int err;
    struct sockdata *sinfo;
    size_t amt;
    char *sptr;
    int val,optlen;
    int ntag,pval;
	Bool audio_configured = NO;

    // tz.tz_minuteswest = 0;
    // tz.tz_dsttime = DST_NONE;

    // timerclear(&tv);
    /* create the socket for listening */

#ifdef DBUG
    cout << "ENTERING sockit() FUNCTION **********\n";
#endif
    if( (s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket");
	  run_status = RT_ERROR;	// Notify inTraverse()
      exit(1);
    }

    /* set up the receive buffer nicely for us */
    optlen = sizeof(char);
    val = sizeof(struct sockdata);
    setsockopt(s, SOL_SOCKET, SO_RCVBUF, &val, optlen);

    /* set up the socket address */
    sss.sin_family = AF_INET;
    sss.sin_addr.s_addr = INADDR_ANY;
    // socknew is offset from MYPORT to allow more than one inst
    sss.sin_port = htons(MYPORT+socknew);

    err = bind(s, (struct sockaddr *)&sss, sizeof(sss));
    if (err < 0) {
      perror("bind");
	  fflush(stdout);
	  run_status = RT_ERROR;	// Notify inTraverse()
	  sleep(1);
	  cout << "\n";
      exit(1);
    }

    listen(s, 1);

    len = sizeof(sss);
    ns = accept(s, (struct sockaddr *)&sss, &len);
    if(ns < 0) {
      perror("accept");
	  run_status = RT_ERROR;	// Notify inTraverse()
      exit(1);
    }
    else {

      sinfo = new (struct sockdata);
      // Zero the socket structure
      sinfo->name[0] = '\0';
      for (i=0;i<MAXDISPARGS;i++) {
		sinfo->data.p[i] = 0;
      }
	  
      // we do this when the -n flag is set, it has to parse rtsetparams()
      // coming over the socket before it can access the values of RTBUFSAMPS,
      // SR, NCHANS, etc.
      if (noParse) {
		
		// Wait for the ok to go ahead
		pthread_mutex_lock(&audio_config_lock);
		if (!audio_config) {
		  if (print_is_on)
			cout << "sockit():  waiting for audio_config . . . \n";
		}
		pthread_mutex_unlock(&audio_config_lock);
		
		while (!audio_configured) {
		  pthread_mutex_lock(&audio_config_lock);
		  if (audio_config) {
			audio_configured = YES;
		  }
		  pthread_mutex_unlock(&audio_config_lock);

		  sptr = (char *)sinfo;
		  amt = read(ns, (void *)sptr, sizeof(struct sockdata));
		  while (amt < sizeof(struct sockdata)) amt += read(ns, (void *)(sptr+amt), sizeof(struct sockdata)-amt);
		  if ( (strcmp(sinfo->name, "rtinput") == 0) ||
			   (strcmp(sinfo->name, "rtoutput") == 0) ||
			   (strcmp(sinfo->name,"set_option") == 0) ||
			   (strcmp(sinfo->name,"bus_config") == 0) ||
			   (strcmp(sinfo->name, "load")==0) ) {
			// these two commands use text data
			// replace the text[i] with p[i] pointers
			for (i = 0; i < sinfo->n_args; i++)
			  strcpy(ttext[i],sinfo->data.text[i]);
			for (i = 0; i < sinfo->n_args; i++) {
			  tmpint = (int)ttext[i];
			  sinfo->data.p[i] = (double)tmpint;
			}
		  }
		  parse_dispatch(sinfo->name, sinfo->data.p, sinfo->n_args, NULL);
		}
		
		if (audio_configured && rtInteractive) {
			if (print_is_on)
				cout << "sockit():  audio set.\n";
		}
		
	  }

// 	  buftime = (double)RTBUFSAMPS/SR;
//       cout << "buftime:  " << buftime << endl;

      // Main socket reading loop
      while(1) {

		sptr = (char *)sinfo;
		amt = read(ns, (void *)sptr, sizeof(struct sockdata));
		while (amt < sizeof(struct sockdata)) amt += read(ns, (void *)(sptr+amt), sizeof(struct sockdata)-amt);
		if ( (strcmp(sinfo->name, "rtinput") == 0) ||
			(strcmp(sinfo->name, "rtoutput") == 0) ||
			(strcmp(sinfo->name,"set_option") == 0) ||
			(strcmp(sinfo->name,"bus_config") == 0) ||
			(strcmp(sinfo->name, "load")==0) ) {
			
			// these two commands use text data
			// replace the text[i] with p[i] pointers
			for (i = 0; i < sinfo->n_args; i++)
				strcpy(ttext[i],sinfo->data.text[i]);
			for (i = 0; i < sinfo->n_args; i++) {
				tmpint = (int)ttext[i];
				sinfo->data.p[i] = (double)tmpint;
			}
		}

	 		// if it is an rtupdate, set the pval array
		if (strcmp(sinfo->name, "rtupdate") == 0) {
		  // rtupdate params are:
		  //	p0 = note tag # 0 for all notes
		  //	p1,2... pn,pn+1 = pfield, value
#ifdef RTUPDATE
		  ntag = (int)sinfo->data.p[0];
		  pthread_mutex_lock(&pfieldLock);
		  for (i = 1; i < sinfo->n_args; i += 2) {
			pval = (int)sinfo->data.p[i];
			pupdatevals[ntag][pval] = sinfo->data.p[i+1];
		  }
		  pthread_mutex_unlock(&pfieldLock);
		  tag_sem=1;
#endif /* RTUPDATE */
		}

		else if ( (strcmp(sinfo->name, "RTcmix_off") == 0) ) {
			printf("RTcmix termination cmd received.\n");
			run_status = RT_SHUTDOWN;	// Notify inTraverse()
 			shutdown(s,0);
			return NULL;
		}
		else if ( (strcmp(sinfo->name, "RTcmix_panic") == 0) ) {
			int count = 30;
			printf("RTcmix panic cmd received...\n");
			run_status = RT_PANIC;	// Notify inTraverse()
			while (count--) {
#ifdef linux
				usleep(1000);
#endif
			}
			printf("Resuming normal mode\n");
			run_status = RT_GOOD;	// Notify inTraverse()
		}
		else {
	
// 		  gettimeofday(&tv, &tz);
// 		  sec = (double)tv.tv_sec;
// 		  usec = (double)tv.tv_usec;
// 		  pthread_mutex_lock(&schedtime_lock);
// 		  schedtime = (((sec * 1e6) + usec) - baseTime) * 1e-6;
// 		  schedtime += ((double)elapsed/(double)SR);
// 		  schedtime += buftime;
// 		  pthread_mutex_unlock(&schedtime_lock);
	  
#ifdef DBUG
// 		  cout << "sockit(): schedtime = " << schedtime << endl;
// 		  cout << "sockit(): buftime = " << buftime << endl;
// 		  cout << "sockit(): baseTime = " << baseTime << endl;
		  cout << "sockit(): elapsed = " << elapsed << endl;
		  cout << "sockit(): SR = " << SR << endl;
#endif
	  
		  // schedtime is accessed in rtsetoutput() to
		  // set the current time.  Plus, in interactive
		  // mode we have to run a slight delay from
		  // "0" or we wind up scheduling events in the past.
	  
		  if(sinfo->name) {
#ifdef ALLBUG
			cout << "SOCKET RECIEVED\n";
			cout << "sinfo->name = " << sinfo->name << endl;
			cout << "sinfo->n_args = " << sinfo->n_args << endl;
			for (i=0;i<sinfo->n_args;i++) {
			  cout << "sinfo->data.p[" << i << "] =" << sinfo->data.p[i] << endl;
			}
#endif
			parse_dispatch(sinfo->name, sinfo->data.p, sinfo->n_args, NULL);
	    
		  }
		}
      }
    }
#ifdef DBUG
    cout << "EXITING sockit() FUNCTION **********\n";
#endif
  }
}
