#include <math.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../../H/sfheader.h"

#define POLE_DEFAULT 24
#define FRAMESIZE_DEFAULT 200
static char lpc_anal[MAXPATHLEN], unstable[MAXPATHLEN], soundfile[MAXPATHLEN];
static int poles = POLE_DEFAULT;
static int framesize = FRAMESIZE_DEFAULT;
static double inskip = 0;
static double duration;
double atof();	/* needed here -- DAS */
static int durset = 0;
static int verbose = 0;

static int first_frame = 1;
static int last_frame = 0;

static int swap;

main(ac,av)
int ac;
char *av[];
	{
	int i, valid_sf, sffd;
	SFHEADER  sfh;
	struct stat sfst;

	/* set up parameters by setting up defaults, reading args,
	/* and overriding defaults as needed 
	*/

	/* have to have at least the nsame of the soundfile */
	if(ac < 2)
		usage();
	strcpy(soundfile, av[ac-1]);
	/* make sure it's valid, and find out the duration so we
	/* can have a default duration 
	*/
	rwopensf(soundfile, sffd, sfh, sfst, av[0], valid_sf,2);
	close(sffd);
	if(valid_sf < 0)
		usage();
	if(verbose)
		printf("srate:%f chans:%d bsize:%d class:%d\n", 
		  sfsrate(&sfh),sfchans(&sfh),sfbsize(&sfst),sfclass(&sfh));
	/* default duration - whole file */
	duration=(double)sfbsize(&sfst)/(double)sfsrate(&sfh)/(double)sfchans(&sfh)/(double)sfclass(&sfh);
	sprintf(lpc_anal, "%s.la", soundfile);
	sprintf(unstable, "%s.uf", soundfile);
	for(i = 1; i < ac && av[i][0] == '-'; i++)
		{
		if(strlen(av[i]) != 2)
			usage();
		if(i >= ac-1)
			usage();
		switch(av[i][1])
			{
			case 'v':
			 verbose = 1;
			 break;
			case 'o':
			 strcpy(lpc_anal, av[++i]);
			 break;
			case 'p':
			 poles = atoi(av[++i]);
		         break;
			case 'i':
			 inskip = atof(av[++i]);
			 break;
			case 'd':
			 duration = atof(av[++i]);
			 durset = 1;
			 break;
			case 'f':
			 framesize = atoi(av[++i]);
			 break;
			default:
			 usage();
			 break;
			}
		}
	/* 'cuz then the soundfile was not at the end */
	if(i != ac-1)
		usage();
	/* now look - if we set inskip but let duration default, then we      
	/* need to keep the durn thing from reading off the end 
	*/
	if(inskip > 0.0 && !durset)
		duration -= inskip;
	create(lpc_anal);
	create(unstable);
	if((last_frame = anallpc(lpc_anal, soundfile, poles, framesize, inskip, duration,verbose)) < 0)
		{
		fprintf(stderr, "fatal error from anallpc\n");
		fprintf(stderr, "BUT I'M GOING ON ANYWAY!\n");

		/* exit(-1); */
		}
#if defined(F2C) && defined(USE_ROOTTEST)
	rootst_(lpc_anal, unstable, &poles, &first_frame, &last_frame);
	stabl_(lpc_anal, unstable, &poles);
	unlink(unstable);
#endif /* F2C */
	fprintf(stderr, "DONE\n");
	}



usage()
	{
	char *use =
"lpc [-o lpc_anal_file] [-p #poles] [-f framesize] \
[-v] [-i inskip] [-d duration] soundfile\n\n\
Defaults: soundfile.la for lpc_analysis file\n\
	  %d for number of poles\n\
	  %d for frame size\n\
	  non-verbose (use -v for verbose)\n\
	  0 for inskip (seconds to skip before readin soundfile)\n\
	  (length of file minus inskip) for duration\n";

	fprintf(stderr, use, POLE_DEFAULT, FRAMESIZE_DEFAULT);
	exit(-1);
	}

create(f)
char *f;
	{
	FILE *fp = NULL;

	if((fp = fopen(f, "w")) == NULL)
		{
		perror(f);
		usage();
		}
	fclose(fp);
	}

