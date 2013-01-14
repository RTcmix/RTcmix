/* lpcheader.c -- lpc data header parsing.  DAS 2/92 */
#include <stdio.h>
#include <sys/file.h>
#include <unistd.h>
#include <ugens.h>
#include "lp.h"
#include "lpcheader.h"

#ifdef MAXMSP
#include <CoreFoundation/CoreFoundation.h>
#endif

LPHEADER analheader;

int
checkForHeader(int afd, int *nPoles, float sr)
{
	int magic[2], headersize=0;
	/* see if second 32 bytes are the lpc header magic number */
	if(read(afd, (char *) magic, sizeof(magic)) != sizeof(magic)) {
		die("dataset", "Can't read analysis file.");
		return -1;
	}
	else lseek(afd, 0, 0);	/* back to beginning */

#ifdef MAXMSP
	if (CFByteOrderGetCurrent() == CFByteOrderLittleEndian) {
		magic[0] = CFSwapInt32BigToHost(magic[0]);
		magic[1] = CFSwapInt32BigToHost(magic[1]);
	}
#endif

	if(magic[1] == LP_MAGIC) {	/* has header */
		if(read(afd, (char *) &analheader, sizeof(analheader))
				!= sizeof(analheader)) {
			die("dataset", "Can't read analysis file header.");
			return -1;
		}

#ifdef MAXMSP
		if (CFByteOrderGetCurrent() == CFByteOrderLittleEndian) {
			analheader.headersize = CFSwapInt32BigToHost(analheader.headersize);
			analheader.lpmagic = CFSwapInt32BigToHost(analheader.lpmagic);
			analheader.npoles = CFSwapInt32BigToHost(analheader.npoles);
			analheader.nvals = CFSwapInt32BigToHost(analheader.nvals);
		}
#endif

		rtcmix_advise("dataset", "This is a csound-type data file with header.");
		if(lseek(afd, analheader.headersize, 0) < 0) {
			die("dataset", "Bad lseek past header.");
			return -1;
		}
		if(*nPoles != 0 && *nPoles != analheader.npoles) {
			die("dataset", "LPC header indicates %d poles--check your numbers.", analheader.npoles);
			return -1;
		}
		else if (analheader.npoles > MAXPOLES) {
			die("dataset", "LPC header %d poles > MAXPOLES (%d)!",
					analheader.npoles, MAXPOLES);
			return -1;
		}
		else if(!*nPoles) /* if none previously specified */
			rtcmix_advise("dataset", "npoles set to %d", *nPoles=analheader.npoles);
		if(sr != 0.0 && sr != analheader.srate) {
			rtcmix_warn("dataset",
				 "Warning: LPC header SR (%.1f) != soundfile SR (%.1f)!", 
				 analheader.srate, sr);
		}
		/* for all future lseeks */
		headersize = analheader.headersize;	
	}
	else if(!*nPoles) {	/* no header and no poles specified */
		die("dataset", "You must specify the number of poles for this file!");
		return -1;
	}
	return headersize;
}
