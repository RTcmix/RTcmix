#include "../../H/sfheader.h"
#include "../../H/ugens.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "carl.h"
#include "procom.h"
#include "defaults.h"
#include <sys/types.h>
#include <sys/stat.h>

#define SIZE 131072 
float array[2][SIZE];
float tabs[2][2];                
float warparray[2][SIZE];
float warptabs[2][2];                
float ringarray[2][SIZE];
float ringtabs[2][2];
float rangearray[2][SIZE];
float rangetabs[2][2];                
float dryarray[SIZE];
float drytabs[2]; 
int haswindow = 0;                 

#define NCMAX 16777216
extern SFHEADER sfdesc[NFILES];

int sferror;
int sinbuf;
int tail;
int	input,output;

/*-------------------------------------------------------
			convolve.c

This program performs fast convolution via the FFT.

-------------------------------------------------------*/

/*
// CMIX datafile parameters
// p[0] = output file time
// p[1] = inputfile skip 
// p[2] = duration 
// p[3] = number of file for impulse response
// p[4] = duration to read file for impuse response file
// p[5] = skip into impulse response file  
// p[6] = gain
// p[7] = invert flag, 1 equals invert
// p[8] = dry signal percentage 
// p[9] = outpan
*/


double 
convolve(p,n_args)
float *p;
int n_args;
{

  float 	*sbuf[2],  		/* array for input and FFT */
  		*tbuf[2],  		/* array for overlap-adding output */
		*filt[2],		/* array for filter transform */
		*dry[2];		/* array for dry signal*/
  
  float		a,		/* temporary */
		b,		/* temporary */
		temp,		/* temporary */
		fmag,
  		srate,		/* sample rate from header */
		gain = 1.,	/* gain */
		max = 0.;	/* maximum */

  long		i,
		j,
		icnt[2],		/* counts input samples */
		ocnt[2], 	/* counts output samples */
		N,		/* FFT size is twice impulse response */
		N2,		/* N / 2 */
		ip,		/* temporary pointer */
		ip1;		/* temporary pointer */

  int		ichan,		/* impulse channels from header */
		invert = 0;	/* flag for deconvolution */

/* bicsf declarations */
  int 		fd,		/* file descriptor */
		result;		/* readopensf() var */
  SFHEADER 	sfh;		/* BICSF header struct */
  struct stat 	sfst;		/* requires /usr/include/sys/stat.h */
  int 		size;		/* packmode size of reference file */

  char 		ch,		/* for crack */
		*cbeg = NULL,
		*cend = NULL,
		*cdur = NULL,
  		*file = "test",
		cbuf[72];	/* buffer for strings to write to header */

/* cmix declarations */
  int		impulse = p[3];		
  int		nsamps=0, isamps=0;
  float 	xin[2],xout[2],dout[2];
  int		c=0,inchan=0,nchnls_out=0;
  int   	ringdown[2],ringtotal;
  int   	infile=1,placeholder=0;
  float   	outpan=0,drymix=0;


	gain    = p[6];
	invert  = p[7];
	drymix  = p[8];
	outpan  = p[9];
	input   = 0;
	output  = 1;

    inchan     = sfchans(&sfdesc[input]);

    ichan = sfchans(&sfdesc[impulse]);

    for (c=0;c<ichan;c++){
   	ocnt[c] = 0;
	icnt[c] = 0;
	}

    nsamps = setnote(p[1],p[2],input);			
    setnote(p[0],p[2],output);				
    nchnls_out = sfchans(&sfdesc[output]);
    if (nchnls_out < 2){
	printf("stereo outfile needed by cmix convolve\n"); 
 	exit(0);
    }	
    isamps     = setnote(p[5],p[4],impulse);	

    size = sizeof(short);
    sinbuf = SF_BUFSIZE / size; 

    if ((isamps) > NCMAX){
      isamps = NCMAX;
      fprintf(stderr,"convolve: warning - impulse response too long\n");
    }

/* Set up buffers. */

  for (N2 = 2; N2 < NCMAX; N2 *= 2)
    if (N2 >= (isamps)) break;
  N = 2 * N2;

  for (c = 0;c<ichan;c++){
    if ((sbuf[c] = (float *) calloc(N+2, sizeof(float))) == NULL)
      malerr("convolvesf: insufficient memory", 1);
    if ((tbuf[c] = (float *) calloc(N2, sizeof(float))) == NULL)
      malerr("convolvesf: insufficient memory", 1);
    if ((filt[c] = (float *) calloc(N+2, sizeof(float))) == NULL)
      malerr("convolvesf: insufficient memory", 1);
    if ((dry[c] = (float *) calloc(N+2, sizeof(float))) == NULL)
      malerr("convolvesf: insufficient memory", 1);
  }
    printf("buffers allocated.....\n");

   for (i=0; i<N2; i++){
       if (GETIN(xin,impulse) <= 0)
         break;
       for (c=0;c<ichan;c++){	
          if (haswindow)
             *(filt[c]+i) =  xin[c] * table(i,array[1],tabs[1]); 
          else *(filt[c]+i) =  xin[c];
     }
   }

/* Finish initialization: fill buffers, take FFT of filt, normalize */
  for (c = 0; c < ichan ; c++){
    for (i=isamps; i<N+2; i++){
      *(filt[c]+i) = 0.;
    }

    rfft(filt[c],N2,1);
    for (i=0; i<=N2; i++){
      a = *(filt[c] + 2*i);
      b = *(filt[c] + 2*i + 1);
      temp = a*a + b*b;
      if (temp > max)
        max = temp;
    }
    if (max != 0.)
      max = gain/(sqrt(max));
    else {
      fprintf(stderr,"convolve: impulse response is all zeros\n");
      exit(1);
    }
    for (i=0; i< N+2; i++)
      *(filt[c] + i) *= max;
  } 
  /* get target file samps */
  for (i=0; i<N2; i++){
     if (cmixgetfloat(xin,inchan) <= 0)
          break;
     if(icnt[c]+i >= nsamps) infile=0;
     for (c=0;c<ichan;c++){	
            if(haswindow){ 
	     *(sbuf[c]+i) =  xin[c] * table(icnt[c],array[0],tabs[0]); 
             *(dry[c]+i)  =  xin[c]; 
	  }
	  else{
             *(sbuf[c]+i) =  xin[c]; 
             *(dry[c]+i)  =  xin[c]; 
          }
      icnt[c]++;
      }
  }
  for (c=0;c<ichan;c++){	
     for (i = icnt[c]; i<N+2; i++){
        *(sbuf[c]+i) = 0.;
        *(dry[c]+i) = 0.;
     }
     if (icnt[c] == 0){
       fprintf(stderr,"convolve: no valid input in source file\n");
       exit(1);
    }
  }
	
  
   
/* Main loop:  Take N-point FFT of next N/2 input samples and multiply
	by FFT of filter impulse response.  Inverse FFT and add first N/2
	resulting samples to last N/2 samples of previous FFT.  */
 ringtotal = N;
 for(c=0;c<ichan;c++){
    ringdown[c] = ringtotal;
    }
  while(ringdown[0] > 0 || ringdown[1] > 0 ){
    for(c=0;c<ichan;c++){
    printf(".");	
      rfft(sbuf[c],N2,1);
      if (!invert) {	/* convolution */
        for (i=0; i<=N2; i++) {
    	  ip = 2*i;
	  ip1 = ip + 1;
	  a = *(sbuf[c]+ip) * *(filt[c]+ip) - *(sbuf[c]+ip1) * *(filt[c]+ip1);
	  b = *(sbuf[c]+ip) * *(filt[c]+ip1) + *(sbuf[c]+ip1) * *(filt[c]+ip);
	  *(sbuf[c]+ip) = a;
	  *(sbuf[c]+ip1) = b;
        }
      }
     else {		/* deconvolution */
      for (i=0; i<=N2; i++) {
	ip = 2*i;
	ip1 = ip + 1;

	if ((fmag =(*(filt[c]+ip)* *(filt[c]+ip)) +
		(*(filt[c]+ip1)* *(filt[c]+ip1))) == 0.) {
	  *(sbuf[c]+ip) = 0.;
	  *(sbuf[c]+ip1) = 0.;
	}
	else {

/*
	  a = (*(sbuf[c]+ip) * *(filt[c]+ip) + *(sbuf[c]+ip1) * *(filt[c]+ip1)) / fmag;
	  b = (*(sbuf[c]+ip) * *(filt[c]+ip1) - *(sbuf[c]+ip1) * *(filt[c]+ip)) / fmag;
	  *(sbuf[c]+ip) = a;
	  *(sbuf[c]+ip1) = b;
*/

	  a = (*(sbuf[c]+ip) / *(filt[c]+ip) + *(sbuf[c]+ip1) / *(filt[c]+ip1));
	  b = (*(sbuf[c]+ip) / *(filt[c]+ip1) - *(sbuf[c]+ip1) / *(filt[c]+ip));
	  *(sbuf[c]+ip) = a;
	  *(sbuf[c]+ip1) = b;
	}
      }
    }

    rfft(sbuf[c],N2,0);
    
    
    for (i=0; i<N2; i++)
      *(tbuf[c]+i) += *(sbuf[c]+i);
  }  /* end chan loop*/  
  if (!invert) {
    for (i=0; i<N2; i++){
      if (++ocnt[0] <= nsamps+N2){
	  for(c=0;c<ichan;c++){ 
	    if(haswindow){
	       xout[c]= *(tbuf[c]+i);
	       dout[c]= *(dry[c]+i) * table(ocnt[0],array[0],tabs[0]);
	    }
	    else{
	       xout[c]=*(tbuf[c]+i);
	       dout[c]=*(dry[c]+i);
	    }
	  }
	  cmixputfloat(xout,dout,drymix,outpan,inchan,ichan);
	}   
      }
      printf(".");
    }
    else {
      for (i=N2-1; i >= 0; i--){
	if (++ocnt[0] <= nsamps+N2){
	  for(c=0;c<ichan;c++){ 
	    if(haswindow){
	       xout[c]= *(tbuf[c]+i);
	       dout[c]= *(dry[c]+i) * table(ocnt[0],array[0],tabs[0]);
	    }
	    else{
	       xout[c]=*(tbuf[c]+i);
	       dout[c]=*(dry[c]+i);
	    }
	  }
        cmixputfloat(xout,dout,drymix,outpan,inchan,ichan);
        }
      }
    }
    for(c=0; c<ichan;c++)
     for (i=0; i<N2; i++)
     *(tbuf[c]+i) = *(sbuf[c]+N2+i);

     /* reset i for second ringdown buffer*/ 
     i=0; 
     if (infile){  
       for (; i<N2; i++) {
         if(cmixgetfloat(xin,inchan) <=0)
	   break;
         if(ocnt[0]+i >= nsamps) infile=0;
	 for (c=0;c<ichan;c++){	
            if(haswindow){ 
	     *(sbuf[c]+i) =  xin[c] * table(icnt[c],array[0],tabs[0]); 
             *(dry[c]+i)  =  xin[c]; 
	    }
	    else{
             *(sbuf[c]+i) =  xin[c]; 
             *(dry[c]+i)  =  xin[c]; 
            }
         icnt[c]++;
         }
     }
    }
     placeholder = i; 
     if (!infile){
       for(c=0; c<ichan;c++){
         for (i=placeholder; i<N2; i++) {
            *(sbuf[c]+i) = 0.;
            *(dry[c]+i) = 0.;
            --ringdown[c];
         }
      }
    printf("\n");
    }
    placeholder = i; 
    for(c=0; c<ichan;c++){
       for (i=placeholder; i<N+2; i++){
         *(sbuf[c]+i) = 0.;
         *(dry[c]+i) = 0.;
       } 
    }
  } /* end of while loop*/
  endnote(output);
  
  printf(">   freeing memory    <\n");
  for(c=0; c<ichan;c++){
     free(sbuf[c]);
     free(tbuf[c]);
     free(filt[c]);
     free(dry[c]);
  }
  return(1);
} /* end convolve */

usage(exitcode)
	int exitcode;
{
	fprintf(stderr,"%s%s%s%s%s%s%s",
	"\nusage: convolvesf [flags] impulse_response_soundfile < floatsams > floatsams\n",
	"\nflags:\n",
	"\tg = gain factor (1.)\n",
	"\tb = begin time in impulse response (first sample to use) (0.)\n",
	"\te = end time in impulse response (last sample to use) (end)\n",
	"\td = duration of impulse response (end - begin)\n",
	"\ti : flag for deconvolution\n\n");
	exit(exitcode);
}

quit()
{
  fprintf(stderr, "exiting\n");
  exit(1); 
}

malerr(str, ex)
  char	*str;
  int	ex;
{
  fprintf(stderr, "%s\n", str);
  fprintf(stderr,"\nconvolvesf: impulse response too long\n");
  exit(ex);
}

int NBYTES=16384;		
profile()
{
	UG_INTRO("convolve",convolve);	        
	UG_INTRO("setwindow",setwindow);	
	UG_INTRO("setwarp",setwarp);		
	UG_INTRO("getwarp",getwarp);		
	UG_INTRO("setring",setring);	
	UG_INTRO("getring",getring);		
	UG_INTRO("setdry",setdry);		
	UG_INTRO("getdry",getdry);		
	UG_INTRO("setrange",setrange);		
	UG_INTRO("getrange",getrange);		

	return(0);
}

cmixgetfloat(xin,inchan)
float *xin[2];
int inchan;
{

GETIN(xin,input);
if (inchan==1)
	xin[1]=xin[0];
return(1);
}

cmixputfloat(xout,dout,drymix,outpan,inchan,ichan)
float *xout,*dout,outpan,drymix;
int inchan,ichan;
{
int c;
float out[2],pan[2],wet;
pan[0]  = sqrt(outpan); 
pan[1]  = sqrt(1-outpan);
wet = (1 - drymix); 

if(ichan==1)
    for(c=0;c<2;c++){
        out[c] = (xout[0]*pan[c])*wet + (dout[0]*pan[c])*drymix;
    }
else{
     for(c=0;c<2;c++)
        out[c] = (xout[c]*pan[c])*wet + (dout[c]*pan[c])*drymix;
    }
    return(ADDOUT(out,output));
}

double 
setwindow(p,n_args)
float *p;
int n_args;
{
    /*
    // table for envelope on read of target file
    // p[0] number of array:   0 = source, 1 = impulse response
    // p[1] duration of window 
    // p[2] -> table values	
    */
    int number = p[0];
    printf("creating window %d\n",number);
    tableset(p[1],SIZE,tabs[number]);
    setline(&p[2],n_args-2,SIZE,array[number]);
    haswindow=1;
}

double
setwarp(p,n_args)
float *p;
int n_args;
{
	/*
	// table for inc. warp 
	// p[0] number of array: 0 = source, 1 = impulse response 
        // p[1] duration of file
        // p[2] -> table values	
	*/
	int number = p[0];
    	printf("creating warp table %d\n",number);
	tableset(p[1],SIZE,warptabs[number]);
        setline(&p[2],n_args-2,SIZE,warparray[number]);
}
double
getwarp(p,n_args)
float *p;
int n_args;
{
	int number,insamp;
	float newskip;

	/*
	// take inskip location and return return  newskip 
	// p[0] number of array: 0 = source, 1 = impulse response 
	// p[1] current inskip
	*/

	number = p[0];
	insamp = (int)(p[1]*44100);
	newskip  = table(insamp,warparray[number],warptabs[number]); 
	return(newskip);
}
double
setrange(p,n_args)
float *p;
int n_args;
{

	/*
	// table for dynamic random ranges 
	// p[0] number of array: 0 = source, 1 = impulse response 
        // p[1] duration of file
        // p[2] -> table values	
	*/
	int number = p[0];
    	printf("creating range table %d\n",number);
	tableset(p[1],SIZE,rangetabs[number]);
        setline(&p[2],n_args-2,SIZE,rangearray[number]);
}
double
getrange(p,n_args)
float *p;
int n_args;
{
	int number,insamp;
	float range;

   	/*	
	// take inskip location and return range value 
	// p[0] number of array: 0 = source, 1 = impulse response 
	// p[1] current inskip
	*/

	number = p[0];
	insamp = (int)(p[1]*44100);
	range  = table(insamp,rangearray[number],rangetabs[number]); 
	return(range);
}
double
setring(p,n_args)
float *p;
int n_args;
{

	/*
	// table for ring 
	// p[0] number of array: 0 = source, 1 = impulse response 
        // p[1] duration of file
        // p[2] -> table values	
	*/

	int number = p[0];
    	printf("creating ring table %d\n",number);
	tableset(p[1],SIZE,ringtabs[number]);
        setline(&p[2],n_args-2,SIZE,ringarray[number]);
}
double
getring(p,n_args)
float *p;
int n_args;
{
	int number;
	long insamp;
	float newindow;

	/*
	// get table value i"newinc," and return inskip + newinc 
	// p[0] number of array: 0 = source, 1 = impulse response 
	// p[1] current inskip
	*/

	number = p[0];
	insamp = (long)(p[1]*44100);
	newindow  = table(insamp,ringarray[number],ringtabs[number]); 
	return(newindow);
}
double
setdry(p,n_args)
float *p;
int n_args;
{

	/*
	// table for dry 
        // p[0] duration of file
        // p[1] -> table values	
	*/

    	printf("creating dry table \n");
	tableset(p[0],SIZE,drytabs);
        setline(&p[1],n_args-1,SIZE,dryarray);

}
double
getdry(p,n_args)
float *p;
int n_args;
{
	int number;
	long insamp;
	float dry;

	/*
	// get table value i"newinc," and return inskip + newinc 
	// p[0] current inskip
	*/

	insamp = (long)(p[0]*44100);
	dry  = table(insamp,dryarray,drytabs); 
	return(dry);
}

