# include "../H/byte_routines.h"
# include <stdio.h>

#define NORM(x,y) ((float) (y) / (float) (x == SF_SHORT ? 32767.0 : 1.0))

#ifndef LINT
static char SccsId[] = "@(#)sndpeak.c	1.7	10/29/85	IRCAM";
#endif

# include <sys/types.h>
# include <sys/stat.h>
# include <sys/file.h>
# include "../H/sfheader.h"

# define CONTINUE error++; continue


/* This program is designed to find the peak absolute value of an existing 
   soundfile and put it in the header */

extern int swap;
int print_is_on;
int init_sound(){};

float maxamp = 0.0;
long totalbytes, maxloc;

main(argc,argv)
int argc;
char *argv[];
{
	int sfd,error = 0;
	struct stat st;
	SFMAXAMP *sfm, *getmaxamp();
	SFHEADER hd;
	int update,i,result,status;
	long sampframes;
	char *name;

	while(--argc) { 
		update = 1;
		name = *++argv;
		printf("soundfile = %s\n",name);

		status = 2;
		rwopensf(name,sfd,hd,st,"sndpeak",i,status);
		if(i<0) closesf();

		/* printf("%f %d %d\n",sfsrate(&hd),sfchans(&hd),sfclass(&hd)); */

		sampframes = sfbsize(&st) / sfchans(&hd) / sfclass(&hd);

		if((sfm = getmaxamp(sfd,update,0,sampframes,&hd)) == NULL) {
			fprintf(stderr,"Bad return from getmaxamp\n");
			CONTINUE;
		}


		/* Unswap main sfheader info */
		if (swap) {
		  printf("Unswapping header info\n");
		  byte_reverse4(&hd.sfinfo.sf_magic);
		  byte_reverse4(&hd.sfinfo.sf_srate);
		  byte_reverse4(&hd.sfinfo.sf_chans);
		  byte_reverse4(&hd.sfinfo.sf_packmode); 
		}      

		/* Swap values before printing */
		if (swap) {
		  printf("Swapping peak amp values so they make sense\n");
		  for (i=0;i<sfchans(&hd);i++) { 
		    byte_reverse4(&(sfm->value[i])); 
		    byte_reverse4(&(sfm->samploc[i]));
		  }
		  byte_reverse4(&(sfm->timetag));
		}


		
		printf("Maxamp for file %s:\n",name);
		for(i = 0; i < sfchans(&hd); i++) {
			printf("channel %d = ",i + 1);
			if(sfmaxamp(sfm,i)) 
				printf("%f (absolute %f) at sample location %d\n",
					NORM(sfclass(&hd),sfmaxamp(sfm,i)),
					sfmaxamp(sfm,i),sfmaxamploc(sfm,i));
					
			else
				printf("silence\n");
		}
		close(sfd);
	}
	exit(error);
}
