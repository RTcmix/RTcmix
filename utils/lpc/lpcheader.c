/* lpcheader.c -- lpc data header parsing.  DAS 2/92 */
#include <stdio.h>
#include <sys/file.h>
#include "lpcheader.h"

LPHEADER analheader;

checkForHeader(afd, nPoles, sr)
int afd, *nPoles;
float sr;
{
	int magic[2], headersize=0;
	/* see if second 32 bytes are the lpc header magic number */
	if(read(afd, (char *) magic, sizeof(magic)) != sizeof(magic)) {
		fprintf(stderr, "Can't read analysis file.\n");
		return -1;
	}
	else lseek(afd, 0, 0);	/* back to beginning */
	if(magic[1] == LP_MAGIC) {	/* has header */
		if(read(afd, (char *) &analheader, sizeof(analheader))
				!= sizeof(analheader)) {
			fprintf(stderr, "Can't read analysis file header.\n");
			return -1;
		}
		printf("This is a csound-type data file with header.\n");
		if(lseek(afd, analheader.headersize, 0) < 0) {
			fprintf(stderr, "Bad lseek past header.\n");
			return -1;
		}
		if(*nPoles != 0 && *nPoles != analheader.npoles) {
			fprintf(stderr, "LPC header indicates %d poles--check your numbers.\n", analheader.npoles);
			return -1;
		}
		else if(!*nPoles) /* if none previously specified */
			printf("npoles set to %d\n", 
				*nPoles=analheader.npoles);
		if(sr != 0.0 && sr != analheader.srate) {
			printf("Warning: LPC header SR (%.1f) != soundfile SR (%.1f)!\n", analheader.srate, sr);
		}
		/* for all future lseeks */
		headersize = analheader.headersize;	
	}
	else if(!*nPoles) {	/* no header and no poles specified */
		fprintf(stderr, "You must specify the number of poles for this file!\n");
		return -1;
	}
	return headersize;
}
