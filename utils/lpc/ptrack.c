#define SFIRCAM
/*  Main program for pitch analysis
 *     The file should be mono.
 *    
 */
 
#include "sysdep.h"
#include <stdio.h>
#ifdef SYS5
#include <fcntl.h>
#endif
#include <sys/file.h>
#ifdef SFIRCAM
#include "../../H/sfheader.h"
#endif /* SFIRCAM */
#include <pwd.h>
#include "crack.h"
#include "ptrack.h"
#include <sys/stat.h>

#include "lpsf.h"		/* INT and FLOAT #defined here */
typedef short SIGTYPE;
#define SAMPTYPE INT		/* shortsam-only */
#define NAMESIZE 1024		/* commandline string size limit */
#define FRAMAX 350

static int LSLICE;
static int JSLIDE;
static float SR;
static float NYQ; 
static int JMAX;
static int MM;
char * buildsfname();
extern	int	remotein;
static int debug = 0, verbose=0;

static int swap;

float gtphi[50][5][18],gtpsi[50][6][18];	/* had to change to globals */
float gtgamph[50][5],gtgamps[50][6]; 		/* hence the 'g' in front of the names */

main(argc, argv)
     int argc;
     char **argv;
{
	SIGTYPE sig[FRAMAX];
	int MIDPOINT,needed,howmuch;
	float freq[50];
	float getpch(),getrms(),lowpass(), ptable();
	float pchlow,pchigh,input[4];
	FILE *pdata;
	int i,n,jj,anal,framesize,result,frameno=0;
	long nsamps;
	float data[2],inskip,dur;
	char name[NAMESIZE], *fullsfname = 0;
	char string[250];
	char *sigp,*sigtemp;
	double rmsm,freqm,rmsx,freqx;
	
#ifdef SFIRCAM
	SFHEADER sfh;
	char sfbuf[SIZEOF_BSD_HEADER], sfname[NAMESIZE];
	int sfd, sfn;
	int Hflag = 0;
	struct stat sfst;

#endif /* SFIRCAM */
	char c;
	int rflag = 0;

	/* DEFAULTS... */
	*sfname = '\0';		/* '\0' => stdin */
	*name = '\0';		/* '\0' => stdout */
	LSLICE = 350;		/* 350 is max! */
	JSLIDE = 200;
	pchlow = 100.0;
	pchigh = 1000.0;
	inskip = 0.0;
	dur = 1.0;		/* should implement an EOF dur! */
	SR = 44100;

	if (debug) verbose++;
	if ( (c = crack(argc, argv, "q", 1)) == 'q' ) {	/* q flag present */
	  verbose++;
	  /* query for everything, ignore any other flags */
	  do {
	    fprintf(stderr,"Enter soundfile name: ");
	  	gets(string);
	  	if(strlen(string)) sscanf(string, "%s",sfname); 
	  	else fprintf(stderr, "You must enter a soundfile name.\n");
	  } while(!strlen(string));
#ifdef SFIRCAM
	  sprintf(name, "n", sfname);
	  fprintf(stderr,"Is soundfile headerless (y or n)? [n] ");
	  gets(string); if(strlen(string)) sscanf(string, "%s",name); 
	  if ( *name == 'y' || *name == 'Y' ) {
	    Hflag++; fprintf(stderr,"Enter sampling rate [%d]: ", SR);
	    scanf("%d",&SR); rflag++;
	  }
#endif /* SFIRCAM */
	  sprintf(name, "%s.pch", sfname);
	  fprintf(stderr,"Enter output file name [%s]: ", name);
	  gets(string); if(strlen(string)) sscanf(string, "%s",name); 

	  fprintf(stderr,"Enter framesize (samps per analyzed frame) [%d]: ",
	  	LSLICE);
	  gets(string); if(strlen(string)) sscanf(string,"%d",&LSLICE);

	  fprintf(stderr,"Enter interframe offset (new samps per frame) [%d]: ",
	  	JSLIDE); 
	  gets(string); if(strlen(string)) sscanf(string,"%d",&JSLIDE); 

	  fprintf(stderr,"Enter low pitch estimate (cps) [%g]: ", pchlow);
	  gets(string); if(strlen(string)) sscanf(string,"%f",&pchlow); 

	  fprintf(stderr,"Enter high pitch estimate [%g]: ", pchigh);
	  gets(string); if(strlen(string)) sscanf(string,"%f",&pchigh); 

	  fprintf(stderr,"Enter initial skip (seconds) [%g]: ", inskip);
	  gets(string); if(strlen(string)) sscanf(string,"%f",&inskip); 

	  fprintf(stderr,"Enter duration of analysis (seconds) [%g]: ", dur);
	  gets(string); if(strlen(string)) sscanf(string,"%f",&dur);
	  
	  /* 	and that's all	 */
	} 
	else if ( c == EOF ) usage("-q flag takes no option");
	else {			/* no q flag present */
	  /* set options from commandline */
	  arg_index = 0;	/* re-crack */
	  while ((c = crack(argc, argv,
#ifdef SFIRCAM
			    "v|r|h|l|f|i|s|d|o|H", 
#else SFIRCAM
			    "r|h|l|f|i|s|d|o|", 
#endif /* SFIRCAM */
			    0)) != NULL) {
	    if (c == EOF) usage("error cracking commandline");
	    switch (c) {	/* cases w/o options */
#ifdef SFIRCAM
	    case 'H': Hflag++; break; /* no soundfile header */
	    case 'v': verbose++; break; /* verbose output */
#endif /* SFIRCAM */
	    default:		/* cases w/ options */
	      if (arg_option == NULL) usage("flag missing an option");
	      switch (c) {
		/* sound flags... */
	      case 'r':		/* sampling rate */
		sscanf(arg_option,"%d", &SR); rflag++; break;
		/* analysis and i/o flags... */
	      case 'h':		/* high cps */
		sscanf(arg_option,"%f", &pchigh); break; 
	      case 'l':		/* low cps */
		sscanf(arg_option,"%f", &pchlow); break; 
	      case 'f':		/* framesize */
		sscanf(arg_option,"%d", &LSLICE); break; 
	      case 'i':		/* framesize */
		sscanf(arg_option,"%d", &JSLIDE); break; 
	      case 's':		/* skiptime */
		sscanf(arg_option,"%f", &inskip); break;	
	      case 'd':		/* duration */
		sscanf(arg_option,"%f", &dur); break; 
	      case 'o':		/* output filename */
		if ( strlen(arg_option) >= NAMESIZE )
		  die("outputfile name too long!");
		strcpy(name,arg_option); break; 
	      default: usage("unrecognized flag");
	      }
	    }
	  }
	  if ( arg_index != argc ) { /* soundfile name here */
	    if ( strlen(argv[arg_index]) >= NAMESIZE )
	      die("soundfile name too long!");
	    strcpy(sfname,argv[arg_index]);
	  }			/* else all done */
	}
	/* check for valid parameters */
	if(LSLICE > FRAMAX)
		dies("\nptrack: frame size too large (%d maximum)", FRAMAX);	
	if ( *name == '\0' ) anal = fileno(stdout); /* write stdout */
	else {
	  if ((anal = creat(name,0644)) < 0) /* create if not exists */
	    dies("ptrack: can't open output data file %s", name);
	}
	if ( *sfname == '\0' ) sfd = fileno(stdin); /* read stdin*/
	else {
/*	  fullsfname = buildsfname(sfname); */
	  fullsfname = sfname;
#ifdef SFREMOTE
	  if (index(fullsfname,':') != NULL)
	    sfd = rsfopen(fullsfname,'<');
	  else 
#endif /* SFREMOTE */
	    if ((sfd = open(fullsfname,O_RDONLY)) < 0)
	      dies("ptrack: can't open soundfile %s",fullsfname);
	}

#ifdef SFIRCAM
	if ( Hflag == 0 ) {	/* use sfheader */
	if(fullsfname == 0)	/* must have specified name if using header */
		usage("You must specify a soundfile name if not using -H.");
	rwopensf(fullsfname,sfd,sfh,sfst,"ptrack",result,2);
	if(result < 0) {
		perror(fullsfname);
		return(-1);
	}
	 /* if ((sfn = readin(sfd,sfbuf,SIZEOF_BSD_HEADER)) < SIZEOF_BSD_HEADER)
	   // die("Ptrack: error reading soundfile header");
	  //sfh = (SFHEADER *) sfbuf; */
	  if (ismagic(&sfh)< 0)
	    dies("ptrack: %s is not a soundfile",sfname);
	  if ( rflag == 0 ) SR = sfsrate(&sfh);
	}
#endif /* SFIRCAM */
	if (verbose) {
	  if (*name == '\0') fprintf(stderr,"Writing to <stdout>\n");
	  else fprintf(stderr,"Writing to %s\n",name);
	  if (*sfname == '\0') fprintf(stderr,"Reading from <stdin>\n");
	  else fprintf(stderr,"Reading from %s\n",sfname);
	  fprintf(stderr,"Sampling rate = %f\n",SR);
	  fprintf(stderr, "Pitch boundary estimates = %f (low) %f (high) \n",
		  pchlow,pchigh);
	  fprintf(stderr,"Framesize = %d\n",LSLICE);
	  fprintf(stderr,"Interframe offset = %d\n",JSLIDE);
	  fprintf(stderr,"rflag: %d Hflag: %d\n",rflag,Hflag);
	}
	{			/* do input skipping */
	  long skipbytes;
	  int nread;
	  skipbytes = (long)(inskip * SR * SAMPTYPE);
	  skipbytes -= skipbytes % 2;
	  if(debug) fprintf(stderr, "%d %f %f\n",skipbytes,inskip,SR);
	  if (remotein || *sfname == '\0') {
	    while (skipbytes > FRAMAX) { /* remote: spinread for lseek */
	      if ((nread = readin(sfd,(char *)sig,FRAMAX)) < FRAMAX)
		die("sound skip error");
	      skipbytes -= nread;
	    }
	    if ((nread = readin(sfd, (char *)sig, skipbytes)) < skipbytes)
	      die("sound skip error");
	  }
	  else if (lseek(sfd,skipbytes,L_INCR) < 0)
	    die("bad sflseek on skip");
	}

	NYQ = SR/2.;
	JMAX = LSLICE/10;
	MM = ((LSLICE/10+1)/2);
	MIDPOINT = LSLICE - JSLIDE;
	sigp = (char *)(sig+MIDPOINT);
	ptable(pchlow,pchigh,gtphi,gtpsi,gtgamph,gtgamps,freq,n); 
	if (debug) fprintf(stderr,"returned\n");

	if ((jj = readin(sfd,(char *)sig,LSLICE*SAMPTYPE)) != LSLICE*SAMPTYPE)
	  die("Ptrack: couldn't fill first frame");
	for(i=0; i<LSLICE; i++) sig[i]=lowpass(sig[i]);
	nsamps = SR * dur;
	while(nsamps) {
	  if (debug) printf("Main loop : Nsamps = %d\n",nsamps);
	  data[0] = getpch(sig,gtphi,gtpsi,gtgamph,gtgamps,freq,n);
	  data[1] = getrms(sig);
	  write(anal,(char *)data,8);  
	  if (verbose) fprintf(stderr,"frame %d: pitch= %.2f rmsamp= %.4f\n",frameno++,data[0],data[1]);
	  sigp = (char *)(sig+MIDPOINT);
	  for(i=0; i<MIDPOINT; i++)  sig[i] = sig[i+JSLIDE];
	  howmuch=0;
	  needed=JSLIDE*SAMPTYPE;

	  if ((howmuch=readin(sfd,sigp,needed)) != needed)
	    break;		/* ran up against eof */

	  for(i=MIDPOINT; i<LSLICE; i++) sig[i] = lowpass(sig[i]);

	  nsamps -= JSLIDE; 
	}
}

usage(msg)
     char *msg;
{
#ifdef SFIRCAM
  fprintf (stderr, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
#else SFIRCAM
  fprintf (stderr, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
#endif /* SFIRCAM */
   "   Usage:  ptrack [flag][option] ... [soundfile]\n",
   "[flag][option] from among:\n",
#ifdef SFIRCAM
   "-rSRATE             sampling rate of sound input (default read from\n",
   "                       soundfile header; else 44100 if -H specified)\n",
#else
   "-rSRATE             sampling rate of sound input (default 44100)\n",
#endif
   "-fFRAMESIZE         analysis framesize in samples (default 350 (max))\n",
   "-iINTERFRAMEOFFSET  offset in samples between frames (default 200 (250 max))\n",
   "-hHIGHPITCH         high pitch estimate in cps (default 1000)\n",
   "-lLOWPITCH          low pitch estimate in cps (default 100)\n",
   "-sSKIPTIME          initial seconds of sound to skip over (default 0.0)\n",
   "-dDURATION          duration in seconds to analyze (default 1.0)\n",
   "-oOUTPUTFILE        analysis output file (stdout if absent, default)\n",
#ifdef SFIRCAM
   "-H                  no soundfile header, SRATE taken from -r flag\n",
   "                       (default reads SRATE from soundfile header)\n",
#endif
   "soundfile           monaural shortsam soundfile\n",
   "                       (reads shorts on stdin if absent, default)\n",
   "\n",
   "      or:  ptrack -q   (queries the user for commandline options)\n",
   "see also:  man ptrack\n\n"
   );
	   die(msg);
}
