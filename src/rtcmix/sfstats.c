#include "../H/sfheader.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#ifdef USE_SNDLIB
#include "../H/sfheader.h"
#endif

void
sfstats(int fd)
{
	int n;
	SFHEADER sfh;
	SFMAXAMP sfm;
	SFCOMMENT sfcm;
	SFLINK sflk;
	char *cp,*getsfcode();

	lseek(fd,0,0);
	rheader(fd,&sfh);

#ifndef USE_SNDLIB
	if (ismagic(&sfh)) {
#endif
		printf("stats for soundfile w/ descriptor %d:\n\n",fd);
       		printf("channels: %d\tsampling rate: %f",sfchans(&sfh),sfsrate(&sfh));
		if (sfclass(&sfh) == SF_SHORT)
			printf("\tinteger file\n");
		else
			printf("\tfloating point file\n");
#ifndef USE_SNDLIB
		}
#endif

#ifndef USE_SNDLIB
	else if (islink(&sfh)) {
		printf("stats for <linked> soundfile w/ descriptor %d:\n\n",fd);
       printf("channels: %d\tsampling rate: %f",sfchans(&sfh),sfsrate(&sfh));
		if (sfclass(&sfh) == SF_SHORT)
			printf("\tinteger file\n");
		else
			printf("\tfloating point file\n");
		}

	else {
		printf("file with file descriptor %d is not a soundfile\n",fd);
		exit(1);
 		}
#endif /* !USE_SNDLIB */

	if (cp = getsfcode(&sfh,SF_MAXAMP)) { 
		bcopy(cp + sizeof(SFCODE), (char *) &sfm, sizeof(SFMAXAMP));
		printf("\nMAXAMP structure found, containing:\n");
		printf("\tchannel #\tmaxamp\tsample #\n");
		for (n = 0; n <= sfchans(&sfh); n++) {
			printf("\t %d\t %f\t %ld\n\n",n,sfmaxamp(&sfm,n),sfmaxamploc(&sfm,n));
			}
		}
	if (cp = getsfcode(&sfh,SF_COMMENT)) {
	    	bcopy(cp + sizeof(SFCODE), (char *) &sfcm, ((SFCODE *)cp)->bsize);
		printf("\nSFCOMMENT structure found, containing:\n");
		printf("  %s\n\n",&sfcomm(&sfcm,0));
		}
#ifndef USE_SNDLIB
	if (cp = getsfcode(&sfh,SF_LINKCODE)) {
		bcopy(cp + sizeof(SFCODE), (char *) &sflk, sizeof(SFLINK));
		printf("\nSFLINK structure found: \n");
		printf("This file is linked to soundfile %s\n",realname(&sflk));
		printf("starting at sample: %d     ending at sample: %d\n\n",startsmp(&sflk),endsmp(&sflk));
		}
#endif /* !USE_SNDLIB */
}
