/* merge.c */

/* This program is used to copy the pitches of a pitch analysis, into
 * the fourth slots of the frames of an lpc analysis.  It assumes only
 * that the lpc frames will have n poles and 4 data values at the beginning
 * of each frame, the fourth of which is the pitch of the frame.
 * In the pitch analysis the frames consists of 2 values, pitch and rms
 * amplitude, so a plot can be done from there or from the concatenated
 * lpc/pitch data set. */

/* modified for command-line options by Luke - 8/98 */

#include <stdio.h>
#include "crack.h"
#ifdef USE_HEADERS
int lphs;	/* header size */
#else
#define lphs (0)
#endif
#define FLOAT 4

int verbose=0;

main(argc, argv)
     int argc;
     char **argv;
{
        int anal,pitches,j;
	int npoles=0,lpcframe,pchframe,pchlast,nbpch;
	long nskiplpc,nskippch,nblpc;
	char *input;
        char *output;
	int argnumber = 0;
	char c;
	float pch[2],val[2];
	if ((c=crack(argc, argv, "q", 1)) == 'q' ) { /* q flag */
	  verbose++;
	}
	if(verbose) {
	printf(" Enter name of lpc analysis file: ");
	scanf("%s",output);
	if((anal = open(output,2)) < 0) {
		fprintf(stderr," Can't open lpc analysis file\n");
		exit(1);
		}
	printf(" Enter number of poles in lpc analysis: ");
	scanf("%d",&npoles);

#ifdef USE_HEADERS
        if((lphs = checkForHeader(anal, &npoles, 0.0)) < 0) exit(1);
#endif /* USE_HEADERS */
	printf(" Enter name of pitch analysis file: ");
	scanf("%s",input);
	if((pitches = open(input,0)) < 0) {
		fprintf(stderr," Can't open pitch analysis file\n");
		exit(1);
		};
	printf(" Enter starting frame in lpc analysis: ");
	scanf("%d",&lpcframe);
	printf(" Enter starting frame in pitch analysis: ");
	scanf("%d",&pchframe);
	printf(" Enter final frame in pitch analysis: ");
	scanf("%d",&pchlast);
	}
	else if (c == EOF ) printf("-q means interactive, silly!\n");
	else {
	  /* get ready for command-line mode */
	  while ((argc > 1) && (argv[1][0] == '-')) {
	    switch(argv[1][1]) {
	    case 'a':
	      output = &argv[1][2];
	      if((anal=open(output, 2)) < 0) {
		fprintf(stderr, "Can't open lpc analysis file\n");
		exit(1);
	      }
	      break;
	      case 'n':
		npoles = atoi(&argv[1][2]);
		#ifdef USE_HEADERS
              if((lphs = checkForHeader(anal, &npoles, 0.0)) < 0) exit(1);
              #endif /* USE_HEADERS */
	      break;
	      case 'p':
		input = &argv[1][2];
		if((pitches = open(input, 0)) < 0) {
		fprintf(stderr, "Can't open pitch analysis file\n");
		exit(1);
	      };
		break;
	      case 'l':
		lpcframe = atoi(&argv[1][2]);
		break;
	      case 's':
		pchframe = atoi(&argv[1][2]);
		break;
	      case 'e':
		pchlast = atoi(&argv[1][2]);
		break;
	      default:
		usage();
		exit(1);
		break;
	    }
	    argv++;
	    argc--;
	  }

	nblpc = (npoles+4)*FLOAT - FLOAT;/*to beginning of next pchloc*/
	nbpch = 2*FLOAT;   /* pch analysis saves 2 float words */
	nskiplpc = lphs + (long)(lpcframe-1)*(long)(nblpc+FLOAT) + (3*FLOAT);  
				 /* pch is 4th data value in fr*/
	if((lseek(anal,nskiplpc,0)) < 0)  {
		printf("Bad lseek on analysis file\n");
		exit(1);
		}
	nskippch = (long)((pchframe-1) * nbpch);
	if((lseek(pitches,nskippch,0)) < 0) {
		printf("Bad lseek on pitch analysis file\n");
		exit(1);
		}
	for(j=pchframe;j<=pchlast;j++) {
		if((read(pitches,(char *)pch,nbpch)) != nbpch) {
			printf("Bad read on pitch analysis file\n");
			exit(1);
			}
		val[0] = pch[0];
		if((write(anal,(char *)val,FLOAT)) != FLOAT) {
			printf("Bad write on lpc analysis file\n");
			exit(1);
			}
		lseek(anal,nblpc,1);
	}
	}
}
usage()
{
  printf("usage:\n\n");
  printf("-q enters interactive mode and will prompt you for info\n\n");
  printf("-a = lpc analysis file\n");
  printf("-n = number of poles\n");
  printf("-p = pitch analysis file\n");
  printf("-l = starting frame in lpc analysis\n");
  printf("-s = starting frame in pitch analysis\n");
  printf("-e = final frame in pitch analysis\n");

}











