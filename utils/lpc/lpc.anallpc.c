#include "../../H/sfheader.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <math.h>

/* analysis program, designed to read only integer, mono 
 * sound files.  Seems to be able to squeeze in 34 pole analysis.
 * Right now it is scaled to a maximum of 34 poles and a maximum
 * segment size of 500, which means that the frame slices must not
 * exceed 250.  On a separate i/d machine these can be comfortably
 * increased.  This program takes about 58k as scaled here. */
/* origin: cmix "anallpc.c" */


/* WARNING _ THESE ARE USED AS EXTERNS BY OTHER FUNCTIONS!   */
/* DON'T CLEAN THEM UP UNLESS YOU *REALLY* KNOW WHAT YOU'RE DOING! */
int NPOLE;
int FRAME;
int NPP1;

#define POLEMAX 100 
#define FLOAT 4
#define FRAMAX   500 
#define NDATA 4 /* number of data values stored with frame */

anallpc(_lpc_anal, _soundfile, _poles, _framesize, _inskip, _duration, verbose)
char *_lpc_anal, *_soundfile;
int _poles, _framesize;
double _inskip, _duration;
int verbose;
{
	int jj,ii,counter;
	SFHEADER sfh;
	int pos;
	float coef[POLEMAX+NDATA],inskip,dur;
	char name[36];
	double errn,rms1,rms2,cc[40];
	short sigs[FRAMAX];
	int sigi[FRAMAX];
	long i,nsamps,begbyte,outskip,nframes;
	short *cyllist;
	int anal,sound,nbytes,nblpc,firstframe,nread;
	struct stat sfst;
	int result;

	if((anal = open(_lpc_anal,2)) < 0) {
		perror(_lpc_anal);
		return(-1);
	}

	rwopensf(_soundfile,sound,sfh,sfst,"anallpc",result,2);
	if(result < 0) {
		perror(_soundfile);
		return(-1);
	}
	if(verbose)
		printsf(&sfh);

	if(sfchans(&sfh) != 1) {
		fprintf(stderr," Can only analyze mono file, sorry\n");
		return(-1);
	}

	if(sfclass(&sfh) != SF_SHORT) {
		fprintf(stderr," Can only analyze integer file\n");
		return(-1);
	}


	NPOLE = _poles;  /* set NPOLE for extern use */
	if(_poles > POLEMAX) {
		fprintf(stderr,"number of poles exceeds maximum allowed\n");
		return(-1);
	}

	FRAME = _framesize * 2;
	if(FRAME > FRAMAX) {
		fprintf(stderr,"exceeds maximum allowed\n");
		return(-1);
	}

	inskip = _inskip;
	dur = _duration;
	begbyte = (long)(inskip * sfsrate(&sfh)) * (long)sfclass(&sfh);
	if(pos = sflseek(sound,begbyte,0) < 0) {
		fprintf(stderr,"Bad sflseek\n");
		return(-1);
	}

	firstframe = 1;
	NPP1 = _poles+1;
	nblpc = (_poles + NDATA)*FLOAT;
	outskip = firstframe * nblpc;
	if((lseek(anal,outskip,0)) < 0) {
		fprintf(stderr,"Bad lseek on analysis file\n");
		return(-1);
	}
	nsamps = dur * sfsrate(&sfh);
	nread = read(sound, (char *)sigs, FRAME * sfclass(&sfh));
        if(nread < 0){
		fprintf(stderr," Bad sfread, nread = %d\n",nread);
		return(-1);
	}


	for(i=0;i<(nread/sfclass(&sfh));i++) 
		sigi[i] = sigs[i];

	nframes = nsamps/_framesize;
	for(i = counter = 0; i < nframes; i++) {
		alpol(sigi,&errn,&rms1,&rms2,cc);
		coef[0] = (float)rms2;
		coef[1] = (float)rms1;
		coef[2] = (float)errn;

		if(verbose)
			printf("%d %f %f %f %f\n",
		    	  counter++,coef[0],coef[1],coef[2],coef[3]);

		coef[3] = 0.;  /*save for pitch of frame */

		for(jj=NDATA; jj<_poles+NDATA; jj++) /* reverse order and change sign */
			coef[jj] = (float)-cc[_poles-jj+(NDATA - 1)];

		for(jj=0; jj<_framesize; jj++) {
			sigi[jj] = sigi[jj+_framesize];
		}

		nread=read(sound,(char *)(sigs+(short)_framesize),_framesize * sfclass(&sfh));
			
		if(nread < 0) {
			fprintf(stderr," bad read, n= %d\n",nread);
			return(-1);
		}
		for(jj=0;jj<(nread/sfclass(&sfh));jj++) 
			sigi[jj+_framesize] = sigs[jj+_framesize];


		if((nbytes = write(anal,(char *)coef,nblpc))!= nblpc) {
			printf(" write error, nbytes = %d\n",nbytes);
			return(-1);
		}
	}
	return(nframes);
}
