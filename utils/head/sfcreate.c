#ifdef USE_SNDLIB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../H/sndlibsupport.h"

/* #define DEBUG */

#ifndef TRUE
 #define TRUE (1)
#endif
#ifndef FALSE
 #define FALSE (0)
#endif

enum {
   UNSPECIFIED_ENDIAN = 0,
   CREATE_BIG_ENDIAN,
   CREATE_LITTLE_ENDIAN
};

#define DEFAULT_SRATE          44100
#define DEFAULT_NCHANS         2
#define DEFAULT_ENDIAN         CREATE_BIG_ENDIAN
#define DEFAULT_IS_SHORT       TRUE
#define DEFAULT_FORMAT_NAME    "aiff"

#define MINSRATE               11025
#define MAXSRATE               96000

#define USAGE_MSG  "\
  option     description                    default                      \n\
  ---------------------------------------------------------------------  \n\
  -r NUM     sampling rate                  [44100]                      \n\
  -c NUM     number of channels             [2]                          \n\
  -i or -f   16-bit integer or 32-bit float [16-bit integer]             \n\
  -b or -l   big-endian or little-endian    [LE for wav, BE for others]  \n\
  -t NAME    file format name; one of...    [aiff, aifc for float]       \n\
             aiff, aifc, wav, next, sun, ircam                           \n\
             (sun is a synonym for next)                                 \n\
                                                                         \n\
  (Defaults take effect unless overridden by supplied values.)           \n\
                                                                         \n\
  NOTE: The following combinations are not available:                    \n\
        aiff and -l                                                      \n\
        aiff and -f (will substitute aifc here)                          \n\
        aifc and -f and -l together (aifc floats are BE only)            \n\
        ircam and -l                                                     \n\
        next and -l                                                      \n\
        wav and -b                                                       \n\
"

char *progname, *sfname = NULL;
int srate = DEFAULT_SRATE;
int nchans = DEFAULT_NCHANS;
int is_short = DEFAULT_IS_SHORT;
int endian = UNSPECIFIED_ENDIAN;
char comment[DEFAULT_COMMENT_LENGTH] = "";

#define FORMAT_NAME_LENGTH 32
char format_name[FORMAT_NAME_LENGTH] = DEFAULT_FORMAT_NAME;

/* these are assigned in check_params */
int header_type;
int data_format;

int main(int, char **);
static int check_params(void);
static void usage(void);


/* Create a soundfile header of the specified type. The usage msg above
   describes the kind of types you can create. (These are ones that
   sndlib can write and that can be useful to the average cmix user.
   There are other kinds of header sndlib can write -- for example,
   for 24-bit files -- but we want to keep the sfcreate syntax as
   simple as possible.

   We allocate a generous comment area, because expanding it after
   sound has already been written would mean copying the entire file.
   (We use comments to store peak stats -- see sndlibsupport.c.)
*/

int
main(int argc, char *argv[])
{
   int     i, nsamps, loc, err;

   /* get name of this program */
   progname = strrchr(argv[0], '/');
   if (progname == NULL)
      progname = argv[0];
   else
      progname++;

   if (argc < 2)
      usage();

   for (i = 1; i < argc; i++) {
      char *arg = argv[i];

      if (arg[0] == '-') {
         switch (arg[1]) {
            case 'c':
               if (++i >= argc)
                  usage();
               nchans = atoi(argv[i]);
               break;
            case 'r':
               if (++i >= argc)
                  usage();
               srate = atoi(argv[i]);
               break;
            case 'i':
               is_short = TRUE;
               break;
            case 'f':
               is_short = FALSE;
               break;
            case 'b':
               endian = CREATE_BIG_ENDIAN;
               break;
            case 'l':
               endian = CREATE_LITTLE_ENDIAN;
               break;
            case 't':
               if (++i >= argc)
                  usage();
               strncpy(format_name, argv[i], FORMAT_NAME_LENGTH - 1);
               format_name[FORMAT_NAME_LENGTH - 1] = 0;  /* ensure termination */
               break;
            default:  
               usage();
         }
      }
      else
         sfname = arg;
   }
   if (sfname == NULL)
      usage();

   if (check_params())
      usage();

   create_header_buffer();

// ***FIXME: see if file exists already ... what to do then?
// CAUTION: writing a header on it will truncate file, I think!!
// Not the familiar cmix behavior.  Use c_write_header_with_fd?

   for (i = 0; i < DEFAULT_COMMENT_LENGTH; i++)
      comment[i] = '\0';

   nsamps = loc = 0;
   err = c_write_header(sfname, header_type, srate, nchans, loc, nsamps,
                                data_format, comment, DEFAULT_COMMENT_LENGTH);

   return 0;
}


/* Convert user specifications into a form we can hand off to sndlib.
   Validate input, and make sure the combination of parameters specifies
   a valid sound file format type (i.e., one whose header sndlib can write).
   See the usage message for the types we allow.
*/
static int
check_params()
{
   int is_aifc = FALSE;

   if (nchans < 1 || nchans > MAXCHANS) {
      fprintf(stderr, "Number of channels must be between 1 and %d.\n\n",
              MAXCHANS);
      exit(1);
   }
   if (srate < MINSRATE || srate > MAXSRATE) {
      fprintf(stderr, "Sampling rate must be between %d and %d.\n\n",
              MINSRATE, MAXSRATE);
      exit(1);
   }

   if (strcasecmp(format_name, "aiff") == 0)
      header_type = AIFF_sound_file;
   else if (strcasecmp(format_name, "aifc") == 0) {
      header_type = AIFF_sound_file;
      is_aifc = TRUE;
   }
   else if (strcasecmp(format_name, "wav") == 0)
      header_type = RIFF_sound_file;
   else if (strcasecmp(format_name, "next") == 0)
      header_type = NeXT_sound_file;
   else if (strcasecmp(format_name, "sun") == 0)
      header_type = NeXT_sound_file;
   else if (strcasecmp(format_name, "ircam") == 0)
      header_type = IRCAM_sound_file;
   else {
      fprintf(stderr, "Invalid file format name: %s\n", format_name);
      fprintf(stderr, "Valid names: aiff, aifc, wav, next, sun, ircam\n");
      exit(1);
   }

   if (endian == UNSPECIFIED_ENDIAN) {
      if (header_type == RIFF_sound_file)
         endian = CREATE_LITTLE_ENDIAN;
      else
         endian = DEFAULT_ENDIAN;
   }

   if (endian == CREATE_BIG_ENDIAN)
      data_format = is_short? snd_16_linear : snd_32_float;
   else
      data_format = is_short? snd_16_linear_little_endian :
                                                   snd_32_float_little_endian;

   /* Check for combinations of parameters we don't allow.
      See the NOTE at bottom of usage message.
   */
   if (header_type == AIFF_sound_file) {
      if (!is_aifc && endian == CREATE_LITTLE_ENDIAN) {
         fprintf(stderr, "AIFF little-endian header not allowed.\n");
         exit(1);
      }
      if (!is_aifc && !is_short) {
         is_aifc = TRUE;
         fprintf(stderr, "WARNING: using AIFC header for floats.\n");
      }
      if (is_aifc && data_format == snd_32_float_little_endian) {
         fprintf(stderr, "AIFC little-endian float header not allowed.\n");
         exit(1);
      }
   }
   if (header_type == IRCAM_sound_file && endian == CREATE_LITTLE_ENDIAN) {
      fprintf(stderr, "IRCAM little-endian header not allowed.\n");
      exit(1);
   }
   if (header_type == NeXT_sound_file && endian == CREATE_LITTLE_ENDIAN) {
      fprintf(stderr, "NeXT little-endian header not allowed.\n");
      exit(1);
   }
   if (header_type == RIFF_sound_file && endian == CREATE_BIG_ENDIAN) {
      fprintf(stderr, "RIFF big-endian header not allowed.\n");
      exit(1);
   }

   /* Tell sndlib which AIF flavor to write. */
   if (header_type == AIFF_sound_file)
      set_aifc_header(is_aifc);

#ifdef DEBUG
   printf("%s: nchans=%d, srate=%d, fmtname=%s, %s, %s\n",
          sfname, nchans, srate, format_name,
          is_short? "short" : "float",
          endian? "bigendian" : "littleendian");
#endif

   return 0;
}


static void
usage()
{
   printf("\nusage: \"%s [options] filename\"\n\n", progname);
   printf("%s\n", USAGE_MSG);
   exit(1);
}



/*****************************************************************************/
#else /* !USE_SNDLIB */
/*****************************************************************************/

#include "../H/byte_routines.h"
#include "../H/sfheader.h"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <signal.h>
#include <errno.h>

int swap;

static SFCODE	ampcode = {
	SF_MAXAMP,
	sizeof(SFMAXAMP) + sizeof(SFCODE)
}; 

static SFCODE	commentcode = {
	SF_COMMENT,
	MINCOMM + sizeof(SFCODE)
	};

#define NBYTES 16384


int check_endian() {
  int i = 1;
  
  if(*(char *)&i) 
    return 0; /* Little endian */
  else
    return 1; /* Big endian */
}


main(argc,argv)

int argc;
char *argv[];

{
	int i,sf,nchars=MINCOMM;
	char *buffer;
#ifndef sgi
	char *malloc();
#endif
	int nbytes,todo;
	int write_big_endian;  /* Defaults to a big endian header */
	int big_endian;
	int tswap;
	int isnative = 0;
	long atol();
	float dur;
	int comment = 0;
	double atof();
	SFHEADER sfh;
	SFMAXAMP sfm;
	SFCOMMENT sfcm;
	FILE *fcom;
	char *sfname,*cp;
	struct stat st;

	write_big_endian = 1;  /* The default */
	swap = 0;  /* This gets set below */
	big_endian = check_endian();



usage:	if(argc < 7) {
		printf("usage: \"sfcreate -r [s. rate] -c [# chans] -[i=int; f=float; m=mu-law] -l[little_endian] -b[big_endian] <-w<x commentsize>> <-d [dur]> filename\"\n");
		exit(1);
		}
	
	dur = 0;

	while((*++argv)[0] == '-') {
		argc -= 2; /* Take away two args */
		for(cp = argv[0]+1; *cp; cp++) {
			switch(*cp) { /* Grap options */
			case 'r': 
				sfsrate(&sfh) = atof(*++argv);
				if(sfsrate(&sfh) < 0 || sfsrate(&sfh) > 60000) {
					printf("Illegal srate\n");
					exit(1);
				}
				printf("Sampling rate set to %f\n",sfsrate(&sfh));
				break;
			case 'd': 
				dur = atof(*++argv);
				printf("Play duration is %f\n",dur);
				break;
			case 'i': 
				sfclass(&sfh) = SF_SHORT;
				break;
			case 'f':
				sfclass(&sfh) = SF_FLOAT;
				break;
			case 'l':
				write_big_endian = 0;
				break;
			case 'b':
			        write_big_endian = 1;  /* Default */
				break;
			case 'c': 
				sfchans(&sfh) = atoi(*++argv);
				if(sfchans(&sfh) != 1 && sfchans(&sfh) != 2 && sfchans(&sfh) != 4) {
					printf("Illegal channel specification\n");
					exit(1);
				}
				printf("Number of channels set to %d\n",sfchans(&sfh));
				break;
			case 'w':
				if(*(argv[0]+2) == 'x') {
					nchars = atoi(*++argv);
					++cp;
					}
				comment = 1;
				break;
			default:  
				printf("Don't know about option: %c\n",*cp);
			}
		}

	}

	/* This is the condition defining when we need to do byte swapping */
	if (((!big_endian) && write_big_endian) || (big_endian && (!write_big_endian))) {
	  swap = 1;
	}

	if((sfsrate(&sfh) == 0.) || (sfclass(&sfh) == 0) 
			|| (sfchans(&sfh) == 0))  {
		printf("********You are missing specifications!\n");
			goto usage;
	}

	/* This looks wierd, no? */
	/* May want to change it just to set the magic # type */
	/*	sfmagic(&sfh) = isnative ? 0 : SF_MAGIC; */
	sfmagic(&sfh) = SF_MAGIC;
        sfname = argv[0];

	if((sf = open(sfname,O_CREAT|O_RDWR,0644)) < 0 ) {
		printf("Can't open file %s\n",sfname);
		exit(-2);
		}
	if(comment && isnative)
		printf("You cannot write a comment into a native file with this program.\n");
	if(isnative) goto writeit;		/* goto  == laziness */

	/* put in peak amps of 0 */
	for(i=0; i<sfchans(&sfh); i++)
		sfmaxamp(&sfm,i)=sfmaxamploc(&sfm,i)=sfmaxamploc(&sfm,i)=0;
	sfmaxamptime(&sfm) = 0;

	/* I wonder if this makes a difference, since it's all 0 right now */
	/* Looks like it doesn't */
	if (swap) {
	  for (i=0;i<sfchans(&sfh);i++) {
	    byte_reverse4(&sfm.value[i]);
	    byte_reverse4(&sfm.samploc[i]);
	  }
	  byte_reverse4(&sfm.timetag);
	}

	/* Note that putsfcode does swapping and unswapping internally for the sfm vector */
	putsfcode(&sfh,&sfm,&ampcode);

	if(!comment) {
		strcpy(&sfcomm(&sfcm,0),sfname);
		sfcomm(&sfcm,strlen(sfname)) = '\n';
		commentcode.bsize = MAXCOMM + sizeof(SFCODE); 

		if (putsfcode(&sfh,&sfcm,&commentcode) < 0) {
			printf("comment didn't get written, sorry!\n");
			exit(-1);
			}
	}
	else {
		system("vi /tmp/comment");
		fcom = fopen("/tmp/comment","r");
		i=0;
		while ( (sfcomm(&sfcm,i) = getc(fcom)) != EOF ) {
			if (++i > MAXCOMM) {
		        	printf("Gimme a break! I can only take %d characters\n",MAXCOMM);
				printf("comment truncated to %d characters\n",MAXCOMM);
				commentcode.bsize = MAXCOMM + sizeof(SFCODE);
				break;
				}
		}
		sfcomm(&sfcm,i) = '\0';
		system("rm /tmp/comment");
		if (nchars > MINCOMM)
				commentcode.bsize = nchars + sizeof(SFCODE);
		if (i > nchars)
				commentcode.bsize = i + sizeof(SFCODE);

		if (putsfcode(&sfh,&sfcm,&commentcode) < 0) {
				printf("comment didn't get written, sorry!\n");
				exit(-1);
				}
	}

writeit:

	if(stat((char *)sfname,&st))  {
	  fprintf(stderr, "Couldn't stat file %s\n",sfname);
	  return(1);
	}
	
	NSchans(&sfh) = sfchans(&sfh);
	NSmagic(&sfh) = SND_MAGIC;
	NSsrate(&sfh) = sfsrate(&sfh);
	NSdsize(&sfh) = (st.st_size - 1024);
	NSdloc(&sfh) = 1024;
	
	switch(sfclass(&sfh)) {
	case SF_SHORT:
	  NSclass(&sfh) = SND_FORMAT_LINEAR_16;
	  break;
	case SF_FLOAT:
	  NSclass(&sfh) = SND_FORMAT_FLOAT;
	  break;
	default:
	  NSclass(&sfh) = 0;
	  break;
	}

	if (swap) {
	  /* Swap the main header info */
	  byte_reverse4(&(sfh.sfinfo.sf_magic));
	  byte_reverse4(&(sfh.sfinfo.sf_srate));
	  byte_reverse4(&(sfh.sfinfo.sf_chans));
	  byte_reverse4(&(sfh.sfinfo.sf_packmode)); 
	  byte_reverse4(&NSchans(&sfh));
	  byte_reverse4(&NSmagic(&sfh));
	  byte_reverse4(&NSsrate(&sfh));
	  byte_reverse4(&NSdsize(&sfh));
	  byte_reverse4(&NSdloc(&sfh));
	  byte_reverse4(&NSclass(&sfh));
	}


	if(wheader(sf,(char *)&sfh)) {
	       printf("Can't seem to write header on file %s\n",sfname);
		perror("main");
		exit(-1);
	}
	
	if(dur) {
		nbytes = todo = dur * sfsrate(&sfh) * (float)sfchans(&sfh)
			 * (float)sfclass(&sfh) + .5;
		fprintf(stderr,"Blocking out file, %d bytes...",nbytes);
		buffer = (char *)malloc(NBYTES);
		/*bzero(buffer,NBYTES);*/
		while(nbytes>0) {
			todo = (nbytes > NBYTES) ? NBYTES : nbytes;
			if(write(sf,buffer,todo) <= 0) {
				printf("Bad write on file\n");
				exit(-1);
			}
			nbytes -= todo;
		}
		printf("\ndone\n");
	}
	putlength(sfname,sf,&sfh);
}

#endif /* !USE_SNDLIB */

