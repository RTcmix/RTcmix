#include <pthread.h>
#include <iostream.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>

#include "../rtstuff/rtdefs.h"
#include "sockdefs.h"
#include "../H/dbug.h"

#include "notetags.h"

extern "C" {
  double parse_dispatch(char*, double*, int);
}

extern "C" int rtInteractive;



extern int noParse;

double schedtime; 	// up here so that rtsetoutput can access this info
// (see below)

extern int socknew; 	// defined in main.C, offset from MYPORT for more than one simultaneous instrument
extern int noaudio;
extern int audio_config;

extern pthread_mutex_t pfieldLock;

extern "C" {
  void *sockit(void*)
  {

    extern double baseTime;
    extern long elapsed;
    double buftime,sec,usec;
    struct timeval tv;
    struct timezone tz;
    char ttext[MAXTEXTARGS][512];
    int i,tmpint;

    // socket stuff
    int s, ns;
    int len;
    struct sockaddr_in sss;
    int err;
    struct sockdata *sinfo;
    int amt,tamt;
    char *sptr;
    int val,optlen;
    double a,b,c;
    int ntag,pval;

    // tz.tz_minuteswest = 0;
    // tz.tz_dsttime = DST_NONE;

    // timerclear(&tv);
    /* create the socket for listening */

    cout << "ENTERING sockit() FUNCTION **********\n";
    if( (s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket");
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

    if( err = bind(s, (struct sockaddr *)&sss, sizeof(sss)) < 0) {
      perror("bind");
      exit(1);
    }

    listen(s, 1);

    len = sizeof(sss);
    ns = accept(s, (struct sockaddr *)&sss, &len);
    if(ns < 0) {
      perror("accept");
      exit(1);
    }
    else {

      buftime = (float)RTBUFSAMPS/SR;
      cout << "buftime:  " << buftime << endl;
	  
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
	cout << "sockit():  waiting for audio config\n";

	sptr = (char *)sinfo;
	amt = read(ns, (void *)sptr, sizeof(struct sockdata));
	while (amt < sizeof(struct sockdata)) amt += read(ns, (void *)(sptr+amt), sizeof(struct sockdata)-amt);
	
	parse_dispatch(sinfo->name, sinfo->data.p, sinfo->n_args);
      }

      buftime = (float)RTBUFSAMPS/SR;

      // Main socket reading loop
      while(1) {

	sptr = (char *)sinfo;
	amt = read(ns, (void *)sptr, sizeof(struct sockdata));
	while (amt < sizeof(struct sockdata)) amt += read(ns, (void *)(sptr+amt), sizeof(struct sockdata)-amt);

	if ( (strcmp(sinfo->name, "rtinput") == 0) || (strcmp(sinfo->name, "rtoutput") == 0) ) { 
	  // these two commands use text data
	  // replace the text[i] with p[i] pointers
	  for (i = 0; i < sinfo->n_args; i++)
	    strcpy(ttext[i],sinfo->data.text[i]);
	  for (i = 0; i < sinfo->n_args; i++) {
	    tmpint = (int)ttext[i];
	    sinfo->data.p[i] = (double)tmpint;
	  }
	}

	if ( (strcmp(sinfo->name, "end") == 0) ) {
	  rtInteractive = 0;
	}
	  
	// if it is an rtupdate, set the pval array
	if (strcmp(sinfo->name, "rtupdate") == 0) {
				// rtupdate params are:
				//	p0 = note tag # 0 for all notes
				//	p1,2... pn,pn+1 = pfield, value
	  ntag = sinfo->data.p[0];
	  pthread_mutex_lock(&pfieldLock);
	  for (i = 1; i < sinfo->n_args; i += 2) {
	    pval = sinfo->data.p[i];
	    pupdatevals[ntag][pval] = sinfo->data.p[i+1];
	  }
	  pthread_mutex_unlock(&pfieldLock);
	  tag_sem=1;
	}

	else {
	
	  gettimeofday(&tv, &tz);
	  sec = (double)tv.tv_sec;
	  usec = (double)tv.tv_usec;
	  schedtime = (((sec * 1e6) + usec) - baseTime) * 1e-6;
	  schedtime += ((double)elapsed/(double)SR);
	  schedtime += buftime;
	  
#ifdef DBUG
	  cout << "sockit(): schedtime = " << schedtime << endl;
	  cout << "sockit(): buftime = " << buftime << endl;
	  cout << "sockit(): baseTime = " << baseTime << endl;
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
	    parse_dispatch(sinfo->name, sinfo->data.p, sinfo->n_args);
	    
	  }
	}
      }
    }
    cout << "EXITING sockit() FUNCTION **********\n";
  }
}
