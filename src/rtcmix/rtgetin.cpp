#include <stdio.h>
#include <unistd.h>
#include "../rtstuff/Instrument.h"
#include "../rtstuff/rtdefs.h"
#include "../H/byte_routines.h"

#ifdef USE_SNDLIB
  #include <stdlib.h>
  #include "../H/sndlibsupport.h"
  extern int inNCHANS;
#endif

extern InputDesc inputFileTable[];

extern int swap;

int rtgetin(float *inarr, Instrument *theInst, int nsmps)
{
	int nbytes;
	int i;
	int fdesc, seeked, sampsRead;
#ifdef USE_SNDLIB
	int n, j, frames;
	static int **inbufs = NULL;
#else
	short in[MAXBUF];

  #ifdef DBUG
	if (swap) {
		printf("!!!!!!!!!!!!SWAPPING!!!!!!!!!\n");
	}
  #endif
#endif /* !USE_SNDLIB */

	if (theInst->fdIndex == AUDIO_DEVICE) {
		for (i = 0; i < nsmps; i++)
			inarr[i] = inbuff[i];
		return 0;
	}

	/* look up file descriptor in table.  It will have been opened by
	 * an earlier call to rtinput().
	 */

	fdesc = inputFileTable[theInst->fdIndex].fd; 

	if (fdesc < 1) { 
		fprintf(stderr, "No input file open for this instrument!\n");
		return -1; 
	}

#ifdef USE_SNDLIB

	if (inbufs == NULL) {    /* 1st time, so allocate array of buffers */
		inbufs = sndlib_allocate_buffers(MAXCHANS, RTBUFSAMPS);
		if (inbufs == NULL) {
			perror("rtgetin: Can't allocate input buffers");
			exit(1);
		}
	}

	/* go to saved file offset for this instrument */

	/* NOTE: clm_seek handles any datum size as if it were a 16bit file.
	 * If we don't care, lseek would be faster.
	 */
	seeked = clm_seek(fdesc, theInst->fileOffset, SEEK_SET);
	if (seeked == -1) {
		fprintf(stderr, "Bad seek on the input soundfile\n");
		return -1;
	}

	frames = nsmps / inNCHANS;
	clm_read(fdesc, 0, frames-1, inNCHANS, inbufs);
//NOTE: doesn't return an error code!!

	for (i = j = 0; i < frames; i++, j += inNCHANS)
		for (n = 0; n < inNCHANS; n++)
			inarr[j+n] = (short)inbufs[n][i];

	/* update our file offset
	 * NOTE: we can't know how much zero padding sndlib did
	 */
	theInst->fileOffset += nsmps * sizeof(short);

	return nsmps;

#else /* !USE_SNDLIB */

	/* go to saved file offset for this instrument */

	seeked = lseek(fdesc, theInst->fileOffset, SEEK_SET);
	if (seeked == -1) {
		fprintf(stderr, "Bad seek on the input soundfile\n");
		return -1;
	}

	nbytes = read(fdesc, (void *)in, nsmps*sizeof(short));

	/* update our file offset */

	if (nbytes > 0)
		theInst->fileOffset += nbytes;

	sampsRead = nbytes / sizeof(short);
	if (sampsRead < 0)
		sampsRead = 0;

	for (i = 0; i < sampsRead; i++) {
		if (swap) {
			byte_reverse2(&in[i]);
		}
		inarr[i] = in[i];
	}

	/* zero out remainder of array */
	for ( ; i < nsmps; i++)
		inarr[i] = 0.0;

	return nbytes;

#endif /* !USE_SNDLIB */

}


