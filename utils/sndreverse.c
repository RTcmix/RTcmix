/* would be nice to have general-purpose splice program.
-s input skip, -o output skip -d dur -r reverse??  */
# include <stdio.h>

#ifndef LINT
static char SccsId[] = "@(#)sndreverse.c	1.4	10/18/85	IRCAM";
#endif

# include <sys/types.h>
# include <sys/stat.h>
# include <sys/file.h>
# include "../H/sfheader.h"
# include "../H/byte_routines.h"

# define SKIP error++; goto skip

/* This program is designed to reverse existing soundfiles.
   The channels of the soundfile are kept in the original order
   contrary to what an analog tape would do. */

int swap;

main(argc,argv)
     int argc;
     char *argv[];
{
  int sfd1,sfd2;
  register int i;
  off_t bytes;
  int readbyte,error = 0;
  SFHEADER hd;
  char *forward,*back;
  struct stat st;
  int backsu,units,nsu,nextsu,result;
  char newname[1024];
  char infile[1024];

  /* Get memory for buffers */

  forward = (char *)malloc(SF_BUFSIZE);
  back = (char *)malloc(SF_BUFSIZE);
  if(!forward || ! back) {
    fprintf(stderr,"Bad allocation for buffers\n");
    exit(1);
  }

  if (argc != 2) {
    fprintf(stderr,"Usage:  %s <filename>\n",argv[0]);
    exit(1);
  }
  
  strcpy(infile,argv[1]);

  readopensf(infile,sfd1,hd,st,"rescale",result);
  if (result < 0) {
    fprintf(stderr,"Error opening file %s\n",infile);
  }

 skip:	printf("%s\n",infile);	
  printf("sndreverse():  skip\n");
  /*
    if(!strncmp("-f",*(argv+1),2)) {
    argc -= 2;
    argv += 2;
    strcpy(newname,*argv);
    }
    else {
  */
  
  strcpy(newname,infile);
  strcat(newname,".r");
  /*
    }
  */
  if(error) {
    error = 0;
  }
  
  if((sfd2 = open(newname,O_CREAT|O_TRUNC|O_WRONLY,0644)) < 0) {
    fprintf(stderr,"Can't create %s\n",newname);
    close(sfd1);
  }
  
  /* Swap main header info */
  if (swap) {
    byte_reverse4(&hd.sfinfo.sf_magic);
    byte_reverse4(&hd.sfinfo.sf_srate);
    byte_reverse4(&hd.sfinfo.sf_chans);
    byte_reverse4(&hd.sfinfo.sf_packmode); 
  }

  if(wheader(sfd2,&hd)) { /* Duplicate header */
    fprintf(stderr,"Write to header failed.\n");
    close(sfd1);
    close(sfd2);
  }

  /* Then swap it back */
  if (swap) {
    byte_reverse4(&hd.sfinfo.sf_magic);
    byte_reverse4(&hd.sfinfo.sf_srate);
    byte_reverse4(&hd.sfinfo.sf_chans);
    byte_reverse4(&hd.sfinfo.sf_packmode); 
  }


  bytes = sfbsize(&st);
  readbyte = bytes % SF_BUFSIZE;
  if(sflseek(sfd1,(long) -readbyte,2) == -1) {
    fprintf(stderr,"Bad seek\n");
    exit(1);
  }

  while(bytes > 0) { /* For all of the samples */
    if((readbyte = read(sfd1,forward,SF_BUFSIZE)) < 0) {
      fprintf(stderr,"Bad read on soundfile %s\n",*argv);
      exit(1);
    }
    /* units = samples per read.
       nsu = number of sample units per read. 
       (sample unit = all channels
       for this sample slot).
       nextsu = next sample unit.
       backsu = current backward sample unit 
       starting channel.	*/

    units = readbyte / sfclass(&hd);
    nextsu = 0;
    nsu = units - sfchans(&hd);
    if(sfclass(&hd) == SF_FLOAT) { /* Do floating point file */
      register float *fbuf = (float *) back;
      float *fforward = (float *) forward;

      while((char *) fbuf < back + readbyte) {
	backsu = nsu - nextsu;
	for(i = 0; i < sfchans(&hd); i++) {
	  *fbuf++ = *(fforward + backsu + i);
	  if (swap) {
	    byte_reverse4(fbuf);
	  }
	}
	nextsu += i;
      }
    }
    else { /* Integer (short) file */
      register short *sbuf = (short *) back;
      short *sforward = (short *) forward;

      while((char *) sbuf < back + readbyte) {
	backsu = nsu - nextsu;
	for(i = 0; i < sfchans(&hd); i++) {
	  *sbuf++ = *(sforward + backsu + i);
	  if (swap) {
	    byte_reverse2(sbuf);
	  }
	}
	nextsu += i;
      }
    }

    if(write(sfd2,back,readbyte) != readbyte) {
      fprintf(stderr,"Bad write on soundfile\n");
      exit(1);
    }
    if((bytes -= readbyte) > 0) 
      if(sflseek(sfd1,(long) -(SF_BUFSIZE + readbyte),1) == -1) {
	fprintf(stderr,"Bad seek\n");
	exit(1);
      }
  }
  close(sfd1);
  close(sfd2);
}

