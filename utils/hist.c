/* now problem is that for >1 chan scan, hist will show || of all channels */
/* ok-->that's not a bug, it's a feature! */
#include "../H/byte_routines.h"
#include "../H/sfheader.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
#include "../H/complexf.h"
#define  BUFSIZE 32768
/* use smaller buffers for hist across network */
#define  ABS(x) ((x < 0) ? (-x) : (x))

SFHEADER sfh;
int loopsize,bytestoread;

extern int swap;

main(argc,argv)

     int argc;
     char *argv[];

{
  static SFMAXAMP sfm;
  struct stat sfst;
  char *cp,*sfname,*getsfcode();
  int i,j,bytes,sf,result;
  int bytenumber,next,total,segments,loopbytes;
  int shutup();
  /*double atof(); */
  char buffer[BUFSIZE]; /* hard wire to keep sloppy users from bad uses*/
  complex s[8192];
  float output[65536];
  float dur;
  int headersize,size;

  float maxamps[4];
  float incr,start,end;
  float jpeak,opeak;
  float scanshort(),scanfloat(),sample;
  float rmsshort(),rmsfloat(),rmsflag;
  int nskips,nprints,nstars,sampleflag;
  int chfirst,chlast,noplot=0;
  short *ipoint;
  float *xpoint;
  int jj;

  if(argc == 1) {
    printf("usage: hist [-p -n nstars] filename\n");
    printf("run-time args: increments [-nsamples], start [-sampletime], dur [-nsamples], [ch_first, ch_last], [overall_peak]\n");
    printf("or: increments, -start, or -end:  plots rms amplitudes\n");
    printf("or: size of fft (power of 2 <=2048), start, channel\n");
    printf("run-time defaults: scan all channels (scan ch 0 for sampl scan), overall_peak read from header\n");
    exit(0);
  }

  nstars = 40;
  while((*++argv)[0] == '-') { 
    argc -= 2; /* Take away two args */
    for(cp = argv[0]+1; *cp; cp++) {
      switch(*cp) { /* Grap options */
      case 'p': 
	noplot = 1;
	break;
      case 'n':
	nstars = atoi(*++argv);
	printf("using %d stars per line, max\n",nstars);
	break;
      }

    }
  }
  sfname = argv[0];
  rwopensf(sfname,sf,sfh,sfst,"hist",result,0);
  if(result < 0) {
    close(sf);
    exit(1);
  }
  headersize = getheadersize(&sfh);
  cp = getsfcode(&sfh,SF_MAXAMP);
  if(cp != NULL)
    bcopy(cp + sizeof(SFCODE), (char *) &sfm, sizeof(SFMAXAMP));

  if (swap) {
    printf("Swapping MAXAMP data\n");
    for (i=0;i<SF_MAXCHAN;i++) {
      byte_reverse4(&sfm.value[i]);
      byte_reverse4(&sfm.samploc[i]);
    }
    byte_reverse4(&sfm.timetag);
  }

  opeak = 0;
  dur = (float)(sfst.st_size - headersize)
    /(float)sfclass(&sfh)/(float)sfchans(&sfh)
    /sfsrate(&sfh);
  printf("duration: %f\n",dur);
  printsf(&sfh);
  fflush(stdout);

  while(1) {
    fprintf(stderr,
	    "Enter increments, starting time, ending time:\t");
    gets(buffer);
    signal(SIGINT, (void *)shutup);
    i = sscanf(buffer,
	       "%f %f %f %d %d %f",
	       &incr,&start,&end,&chfirst,&chlast,&opeak);
    if(i == 3) {
      chfirst = 0; 
      chlast = sfchans(&sfh) - 1;
    }
    if(i == 4) chlast = chfirst;
    if(i < 6) {
      for(i=chfirst; i<=chlast; i++)
	if(sfmaxamp(&sfm,i) > opeak) 
	  opeak = sfmaxamp(&sfm,i);
    }
    if(!opeak) opeak = 32678;  /* default for files with no amp */

    /* if increments is < 0 it means specify incr in samplesize
       if starting time is < 0 it means sample number
       if duration < 0 it indicates number of samples to look at
       if incr >= 32 we are doing fft, end arg is now channel # */

    rmsflag = 0;
    if((incr > 0) && ((start < 0 ) || (end < 0))) {
      rmsflag = 1; 
      start = ABS(start);
      end = ABS(end);
      printf("plotting maximum rms amplitudes\n");
    }
    sampleflag = (incr < 0) ? 1 : 0;
    if(incr < 0.) incr = -incr/sfsrate(&sfh);
    if(start < 0.) start = -start/sfsrate(&sfh);
    if(end < 0.) end = start - end/sfsrate(&sfh);

    bytes = start * sfclass(&sfh) * sfchans(&sfh) * sfsrate(&sfh);
    bytes -= bytes % (sfclass(&sfh) * sfchans(&sfh)); 

    /* roundout to multiple of sampleblocksize */ 
    loopsize = (incr < 32) ? incr * sfsrate(&sfh) + .5 : incr;
    loopbytes = loopsize * sfclass(&sfh) * sfchans(&sfh);
    if((bytenumber = sflseek(sf,bytes,0)) < 0) {
      printf("bad lseek on file %s\n",sfname);
      exit(-1);
    }
    bytenumber -= headersize;
    bytestoread = (incr < 32) ? 
      sfsrate(&sfh) * sfclass(&sfh) * sfchans(&sfh) * (end-start):
      incr * sfclass(&sfh) * sfchans(&sfh);
		    
    while(bytestoread > 0) {
      total = next = 
	(bytestoread > loopbytes) ? loopbytes : bytestoread;
      for(i=0; i<sfchans(&sfh);i++) maxamps[i] = 0;
      while(next > 0) {
	segments = (next > BUFSIZE) ? BUFSIZE : next;
	if((read(sf,buffer,segments)) != segments) {
	  fprintf(stderr,"Reached eof!\n");
	  goto nextstep;
	}
	bytenumber += segments;
	if(sfclass(&sfh) == SF_SHORT) 
	  sample = rmsflag ? 
	    rmsshort(buffer,segments,maxamps,chfirst,chlast) : 
	  scanshort(buffer,segments,maxamps,chfirst,chlast);
	else
	  sample = rmsflag ? 
	    rmsfloat(buffer,segments,maxamps,chfirst,chlast) : 
	  scanfloat(buffer,segments,maxamps,chfirst,chlast);
	next -= segments;
      }
      bytestoread -= total;
      if(sampleflag) {
	/* print with + & - along central axis */
	printf("%6d %+e ",(bytenumber-segments)/
	       sfclass(&sfh)/sfchans(&sfh),sample);
	if(!noplot) {
	  nprints = 
	    (float)nstars/2 * ABS(sample)/opeak;
	  nskips = (sample < 0.) ? 
	    nstars/2 - nprints : nstars/2;
	  printf("\t");
	  for(j=0;j<nskips;j++) printf(" ");
	  for(j=0;j<nprints;j++) printf("*");
	}
	printf("\n");
	fflush(stdout);
      }
      else if(incr >= 32) {
	size = (int)incr; 
	sortout(buffer,size,end);
	if(sfclass(&sfh) == SF_SHORT)  {
	  ipoint = (short *)buffer;
	  for(jj=0; jj<size; jj++)
	    s[jj].re = ipoint[jj]; 
	}
	else	{
	  xpoint = (float *)buffer;
	  for(jj=0; jj<size; jj++)
	    s[jj].re = xpoint[jj];
	  s[jj].im = 0;
	}
	fft(1,size,s);
	for(jj=0; jj<size; jj++)
	  output[jj] = s[jj].re;
	fftprint(output,size);
	goto nextstep;
      }
      else {
	jpeak = 0;
	printf("%9.4f ",start);
	start += incr;
	for(i=chfirst; i<=chlast; i++) { 
	  printf("%+e ",maxamps[i]);
	  jpeak = (maxamps[i] > jpeak) ? 
	    maxamps[i] : jpeak;
	}
	if(!noplot) {
	  nprints = (float)nstars * jpeak/opeak;
	  if(rmsflag) nprints *= 2;
	  if(nprints <= nstars)
	    for(j=0;j<nprints;j++) printf("*");
	  else for(j=0;j<nstars;j++) printf(">");
	}
	printf("\n");
	fflush(stdout);
      }
    }
  nextstep:	
    fflush(stdout);	/*  RK was here	*/
    signal(SIGINT, SIG_DFL); 
  }
}
float rmsshort(buffer,segments,maxamps,chfirst,chlast)
     float *maxamps;
     short *buffer;
{
  int i,j,sampleno,samplesize;
  samplesize = segments / (SF_SHORT * sfchans(&sfh));
  *maxamps = *(maxamps + 1) = 0;
  for(i=0,sampleno=0; i<samplesize; i += sfchans(&sfh)) {
    for(j=chfirst; j<=chlast; j++) {
      if (swap) {
	byte_reverse2(&buffer[sampleno+j]);
      }    
      maxamps[j] += buffer[sampleno+j] * buffer[sampleno+j];
    }
    sampleno += sfchans(&sfh);
  }
  for(j=chfirst; j<=chlast; j++) 
    maxamps[j] = sqrt(maxamps[j]/(float)samplesize);
  return((float)buffer[chfirst]); /* return first value for sample scan*/
}

float rmsfloat(buffer,segments,maxamps,chfirst,chlast)
     float *maxamps;
     float *buffer;
{
  int i,j,sampleno,samplesize;
  samplesize = segments / (SF_FLOAT * sfchans(&sfh));
  *maxamps = *(maxamps + 1) = 0;
  for(i=0,sampleno=0; i<samplesize; i += sfchans(&sfh)) {
    for(j=chfirst; j<=chlast; j++) {
      if (swap) {
	byte_reverse4(&buffer[sampleno+j]);
      }    
      maxamps[j] += buffer[sampleno+j] * buffer[sampleno+j];
    }
    sampleno += sfchans(&sfh);
  }
  for(j=chfirst; j<=chlast; j++)  
    maxamps[j] = sqrt(maxamps[j]/(float)samplesize);
  return(buffer[chfirst]);
}

float scanshort(buffer,segments,maxamps,chfirst,chlast)
     float *maxamps;
     short *buffer;
{
  int i,j,sampleno,samplesize;
  samplesize = segments /SF_SHORT;
  for(i=0,sampleno=0; i<samplesize; i += sfchans(&sfh)) {
    for(j=chfirst; j<=chlast; j++) {
      if (swap) {
	byte_reverse2(&buffer[sampleno+j]);
      }
      if(ABS(buffer[sampleno+j]) > maxamps[j]) {
	maxamps[j] = ABS(buffer[sampleno+j]);
      }
    }
    sampleno += sfchans(&sfh);
  }
  return((float)buffer[chfirst]); /* return first value for sample scan*/
}
float scanfloat(buffer,segments,maxamps,chfirst,chlast)
     float *maxamps;
     float *buffer;
{
  int i,j,sampleno,samplesize;
  samplesize = segments/SF_FLOAT;
  for(i=0,sampleno=0; i<samplesize; i += sfchans(&sfh)) {
    for(j=chfirst; j<=chlast; j++) {
      if (swap) {
	byte_reverse4(&buffer[sampleno+j]);
      }    
      if(ABS(buffer[sampleno+j]) > maxamps[j]) {
	maxamps[j] = ABS(buffer[sampleno+j]);
      }
    }
    sampleno += sfchans(&sfh);
  }
  return(buffer[chfirst]);
}
scanfloat2(buf,segments,maxamps,chfirst,chlast)
     float *maxamps;
     char *buf;
{
  register float *fbuffer = (float *) buf;
  long samples;
  float val;
  register int i;
  int chans;

  samples = segments/SF_FLOAT;
  chans = sfchans(&sfh);

  while(samples > 0) {
    for(i = chfirst; i <=chlast; i++) {
      val = ABS(*fbuffer);
      if(val > maxamps[i]) {
	maxamps[i] = val;
      }
      fbuffer++;
    }
    samples -= chans;
  }
}

shutup() 
{
  bytestoread = 0;
}

sortout(buffer,size,end)
     char *buffer;
     float end;
{

  /* sort out relevant channel and remove dcbias */

  float *fpoint,aver;
  short *ipoint;
  int i,j; /* end is actually the number of the channel to look at */
  int channel = (int)end + .01;

  if(sfclass(&sfh) == 2) {
    ipoint = (short *)buffer;
    for(i=0,j=channel,aver=0; i<size; i++, j += sfchans(&sfh)) 
      aver += ipoint[i] = ipoint[j];
    aver /= (float)size;
    for(i=0; i<size; i++) ipoint[i] -= aver;
  }
  else {
    fpoint = (float *)buffer;
    for(i=0,j=channel,aver=0; i<size; i++, j += sfchans(&sfh)) 
      aver += fpoint[i] = fpoint[j];
    aver /= (float)size;
    for(i=0; i<size; i++) fpoint[i] -= aver;
  }
}

fftprint(output,size)
     float *output;
{
  int i,j;
  float peak=0;
  float nstars = 40.;
  bytestoread = 1;
  for(i=0; i<size/2; i++) 
    if(output[i] > peak) peak = output[i];
  for(i=0; i<size/2; i++) {
    printf("%6.0f %6.0f",
	   sfsrate(&sfh)*(float)i/(float)size,output[i]);
    for(j=0; j<(nstars * output[i]/peak); j++) printf("*");
    printf("\n");
    if(!bytestoread) return;
  }
}
