/* fast program to rescale from floating point file to integer file.
   It will create an integer file called  filename.short if you don't
   specify an output file. */
/* could also make this into a neat general purpose alteration program ?*/

/* TODO create option to skip on input and output, and specify duration and 
   optional peak */

#include "../H/byte_routines.h"
#include "../H/sfheader.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#define  BUFSIZE 32768

static SFCODE	ampcode = {
  SF_MAXAMP,
  sizeof(SFMAXAMP) + sizeof(SFCODE)
}; 

static SFCODE	commentcode = {
  SF_COMMENT,
  MINCOMM + sizeof(SFCODE)
};


extern int swap;

void main(int argc, char *argv[])
{
  SFMAXAMP sfm,sfmnew;
  SFCOMMENT sfcm;
  SFHEADER sfh1,sfh2;
  SFCODE *sizer;
  struct stat sfst1,sfst2,st;
  
  char *cp,*getsfcode();
  char *sfin,*sfout;
  double atof();
  short outbuffer[BUFSIZE];
  short s_inbuffer[BUFSIZE];
  float f_inbuffer[BUFSIZE];
  int sf1,sf2;
  int i,n,bytes,words,inbytes,outbytes,result,readbytes,durbytes;
  int replace,empty,skipbytes,outskipbytes,newfile;
  float skpin,dur,newpeak,outskip;
  char command[256];
  char newname[128];
  char *point,*point2,*strcat();
  short *bufp;
  double Oresult = 32766;
  float tsamp;
  double opeak,factor;
  int t_isamp;
  
  factor = 0;
  if(argc == 1) {
  usage:		printf("usage: -s inskip -d dur -o outskip -p peakamp -P desired peak -r [write over floating-point file] -e emptybuffers_at_end -z [create new file] -f factor inputfile [outputfile]\n");
  printf("defaults: s=0, d=to_eof, o=0, p=input_peak, e=1, P=32767, f=P/input_peak write over old file, outputfile=inputfile.short\n");
  exit(0);
  }
  replace=newfile=skpin=dur=newpeak=outskip=factor=0;
  
  empty = 1;  /* defaults to 1 empty buffers at end*/
  
  while((*++argv)[0] == '-') {
    argc -= 2; /* Take away two args */
    for(cp = argv[0]+1; *cp; cp++) {
      switch(*cp) { /* Grap options */
      case 'r':
	replace = 1;
	printf("Writing over floating-point file\n");
	break;
      case 's': 
	skpin = atof(*++argv);
	printf("input skip = %f\n",skpin);
	break;
      case 'd': 
	dur = atof(*++argv);
	printf("rescale duration = %f\n",dur);
	empty = 0; /* no empty buffers for splice */
	break;
      case 'f':
	factor = atof(*++argv);
	printf("specified factor = %f\n",factor);
	break;
      case 'p': 
	newpeak = atof(*++argv);
	printf("specified peak = %f\n",newpeak);
	break;
      case 'P':
	Oresult = atof(*++argv);
	printf("resultant peak = %f\n",Oresult);
	break;
      case 'o': 
	outskip = atof(*++argv);
	printf("output skip =  %f\n",outskip);
	break;
      case 'e':
	empty = atof(*++argv);
	printf("write %d empty buffers at end\n",empty);
	break;
      case 'z':
	newfile = 1;
	break;
      default:
	printf("uh-oh, unkown option\n");
	goto usage;
      }
    }
  }
  sfin = (char *)malloc(strlen(argv[0]));
  strcpy(sfin,argv[0]);
  
  if (argv[1]) {
    sfout = (char *)malloc(strlen(argv[1]));
  }
  sfout = argv[1];
  readopensf(sfin,sf1,sfh1,sfst1,"rescale",result);
  if(result < 0) {
    close(sf1);
    exit(1);
  }
  printf("rescale():  sfin = %s\n",sfin);

  /* do input skip on input file*/
  if(skipbytes = 
     skpin * sfclass(&sfh1) * sfsrate(&sfh1) * sfchans(&sfh1)) {
    skipbytes -= skipbytes % (sfclass(&sfh1) * sfchans(&sfh1));
    /* make sure it lands on sample block */
    if(sflseek(sf1,skipbytes,0) == -1) {
      printf("Bad skip on input file\n");
      exit(1);
    }
  }
  
  printsf(&sfh1);
  if(sfclass(&sfh1) == SF_SHORT) {
    printf("Note: Input file has short integers.\n");
  }
  
  cp = getsfcode(&sfh1,SF_MAXAMP);
  
  bcopy(cp + sizeof(SFCODE), (char *) &sfm, sizeof(SFMAXAMP));
  
  if (swap) {
    printf("Swapping MAXAMP data\n");
    for (i=0;i<SF_MAXCHAN;i++) {
      byte_reverse4(&sfm.value[i]);
      byte_reverse4(&sfm.samploc[i]);
    }
    byte_reverse4(&sfm.timetag);
  }
  
  printsf(&sfh1);
  
  for(i=0,opeak=0; i<sfchans(&sfh1); i++) 
    if(sfmaxamp(&sfm,i) > opeak) opeak = sfmaxamp(&sfm,i);
  opeak = newpeak ? newpeak : opeak;
  if(!opeak) {
    printf("Sorry, but I have no peak amplitude for this file.\nPut one in with sfhedit, sndpeak, or use the -p flag in rescale.\n");
    close(sf1);
    exit(-3);
  }
  printf("Peak amplitude of input file is %f\n",opeak);
  
  if(!factor) factor = (float)(Oresult/opeak);
  printf("factor = %f\n",factor);
  
  if((cp=getsfcode(&sfh1,SF_COMMENT)))  {
    sizer = (SFCODE *) cp;
    bcopy(cp + sizeof(SFCODE) , (char *) &sfcm, sizer->bsize);
  }
  
  /* REPLACE */
  if(replace) {
    sfout = sfin;
  }

  newrwopensf(sfout,sf2,sfh2,sfst2,"rescale",result,2);

  if(result < 0) {
    if(sfout == NULL) {
      point = sfin;
      if(sfclass(&sfh1) == SF_FLOAT) 
	point = strcat(sfin,".short");
      else
	point = strcat(sfin,".xshort");
    }
    else
      point = sfout;

    sfmagic(&sfh2) = SF_MAGIC;
    sfclass(&sfh2) = SF_SHORT;
    sfchans(&sfh2) = sfchans(&sfh1);
    sfsrate(&sfh2) = sfsrate(&sfh1);

    if((sf2 = open(point,O_CREAT|O_RDWR,0644)) < 0 ) {
      printf("Can't open file %s\n",point);
      exit(-2);
    }
    if(newfile) ftruncate(sf2,SIZEOF_BSD_HEADER);
    
    for(i=0; i<sfchans(&sfh2); i++) {
      sfmaxamp(&sfmnew,i)=sfmaxamp(&sfm,i)*factor;
      sfmaxamploc(&sfmnew,i)=sfmaxamploc(&sfm,i);
    }
    sfmaxamptime(&sfmnew) = sfmaxamptime(&sfm);
    
    if (swap) {
      for (i=0;i<SF_MAXCHAN;i++) {
	byte_reverse4(&sfmnew.value[i]);
	byte_reverse4(&sfmnew.samploc[i]);
      }
      byte_reverse4(&sfmnew.timetag);
    }
    
    putsfcode(&sfh2,&sfmnew,&ampcode);
    if (putsfcode(&sfh2,&sfcm,&commentcode) < 0) {
      printf("comment didn't get written, sorry!\n");
      exit(-1);
    }
    printf
      ("\nCreating output file: %s\n",point);

    sfclass(&sfh2) = SF_SHORT;
    
    /* Need to write out NeXT data */
    if(stat(point,&st))  {
      fprintf(stderr, "putlength:  Couldn't stat file %s\n",sfout);
      return(1);
    }
    printf("Copying IRCAM data into Next header\n");
    NSchans(&sfh2) = sfh2.sfinfo.sf_chans;
    NSmagic(&sfh2) = SND_MAGIC;
    NSsrate(&sfh2) = sfh2.sfinfo.sf_srate;
    NSdsize(&sfh2) = (int)(st.st_size - 1024);
    NSdloc(&sfh2) = 1024;
    
    switch(sfh2.sfinfo.sf_packmode) {
    case SF_SHORT:
      NSclass(&sfh2) = SND_FORMAT_LINEAR_16;
      break;
    case SF_FLOAT:
      NSclass(&sfh2) = SND_FORMAT_FLOAT;
      break;
    default:
      NSclass(&sfh2) = 0;
      break;
    }
    if (swap) {
      byte_reverse4(&NSchans(&sfh2));
      byte_reverse4(&NSmagic(&sfh2));
      byte_reverse4(&NSsrate(&sfh2));
      byte_reverse4(&NSdsize(&sfh2));
      byte_reverse4(&NSdloc(&sfh2));
      byte_reverse4(&NSclass(&sfh2));
    }

    /* Don't forget to swap the header data ! */
    if (swap) {
      byte_reverse4(&(sfh2.sfinfo.sf_magic));
      byte_reverse4(&(sfh2.sfinfo.sf_srate));
      byte_reverse4(&(sfh2.sfinfo.sf_chans));
      byte_reverse4(&(sfh2.sfinfo.sf_packmode)); 
    }

    /* Write out the header */
    if(wheader(sf2,(char *)&sfh2)) {
      printf("Can't seem to write header on file %s\n"
	     ,point);
      perror("main");
      exit(-1);
    }

    /* Then swap it back! */
    if (swap) {
      /* Swap the main header info */
      byte_reverse4(&(sfh2.sfinfo.sf_magic));
      byte_reverse4(&(sfh2.sfinfo.sf_srate));
      byte_reverse4(&(sfh2.sfinfo.sf_chans));
      byte_reverse4(&(sfh2.sfinfo.sf_packmode)); 
    }
  }
  else if(!replace) printsf(&sfh2);
  if(!replace && (sfclass(&sfh2) != SF_SHORT)) {
    printf("Output file must have short integers.\n");
    exit(-1);
  }
  
  /* do output skip*/
  if(outskipbytes = 
     outskip * sfclass(&sfh2) * sfsrate(&sfh2) * sfchans(&sfh2)) {
    outskipbytes -= 
      outskipbytes % (sfclass(&sfh2) * sfchans(&sfh2));
    /* make sure it lands on sample block */
    if(sflseek(sf2,outskipbytes,0) == -1) {
      printf("Bad skip on output file\n");
      exit(1);
    }
  }
  
  readbytes = inbytes = BUFSIZE * sfclass(&sfh1);
  
  durbytes = dur * sfclass(&sfh1) * sfchans(&sfh1) * sfsrate(&sfh1);
  
  fflush (stdout);
  fprintf(stderr,"Rescaling....\t");
  
  while(1) {
    if(dur) {
      inbytes = (durbytes > readbytes) ? inbytes : durbytes;
      durbytes -= inbytes;
    }

    if (sfclass(&sfh1) == SF_FLOAT) {
      if((bytes = read(sf1,f_inbuffer,inbytes)) <= 0) {
	printf("reached eof on input\n");
	close(sf1);
	break;
      }
    }
    if (sfclass(&sfh1) == SF_SHORT) {
      if((bytes = read(sf1,s_inbuffer,inbytes)) <= 0) {
	printf("reached eof on input\n");
	close(sf1);
	break;
      }
    }
    
    words = bytes/sfclass(&sfh1);
    outbytes = words * SF_SHORT;
    
    printf("factor:  %f\n",factor);
    if(sfclass(&sfh1) == SF_SHORT) {
      for(i=words-1; i>=0; i--) {
 	if (swap) {
	  t_isamp = (short)s_inbuffer[i];
	  s_inbuffer[i] = (short)reverse_int2(&t_isamp);
 	}
	outbuffer[i] = (short) (s_inbuffer[i] * factor);
 	if (swap) {
	  t_isamp = outbuffer[i];
	  outbuffer[i] = reverse_int2(&t_isamp);
 	}
      }
    }
    else {
      for(i=words-1; i>=0; i--) {
	if (swap) {
	  byte_reverse4(&f_inbuffer[i]); 
	}
	outbuffer[i] = (short)(f_inbuffer[i] * factor);
	if (swap) {
	  byte_reverse4(&outbuffer[i]);
	}
      }
    }
    
    if(write(sf2,(char *)outbuffer,outbytes) != outbytes) {
      printf("Bad write on output file\n");
      close(sf1);
      close(sf2);
      exit(0);
    }
  }
  
  /* write  empty buffers */
  for(i=0; i<words; i++) outbuffer[i] = 0;
  for(i=0; i<empty; i++) {
    if(n=write(sf2,(char *)outbuffer,outbytes) != outbytes) {
      printf("Bad write on output file\n");
      close(sf2);
      exit(0);
    }
  }

  if(replace) {
    i = lseek(sf2,0,1);
    if(ftruncate(sf2,i) < 0) 
      printf("Bad truncation\n");
    lseek(sf2,0,0);
    
    for(i=0; i<sfchans(&sfh2); i++) {
      sfmaxamp(&sfmnew,i)=sfmaxamp(&sfm,i)*factor;
      sfmaxamploc(&sfmnew,i)=sfmaxamploc(&sfm,i);
    }
    sfmaxamptime(&sfmnew) = sfmaxamptime(&sfm);
    
    putsfcode(&sfh2,&sfmnew,&ampcode);
    sfclass(&sfh2) = SF_SHORT;
    
    /* Need to write out NeXT data */
    /* Need to get file size for dataLocation value */
    if(stat(sfout,&st))  {
      fprintf(stderr, "putlength:  Couldn't stat file %s\n",sfout);
      return(1);
    }
    printf("Copying IRCAM data into Next header\n");
    NSchans(&sfh2) = sfh2.sfinfo.sf_chans;
    NSmagic(&sfh2) = SND_MAGIC;
    NSsrate(&sfh2) = sfh2.sfinfo.sf_srate;
    NSdsize(&sfh2) = (int)(st.st_size - 1024);
    NSdloc(&sfh2) = 1024;
    
    switch(sfh2.sfinfo.sf_packmode) {
    case SF_SHORT:
      NSclass(&sfh2) = SND_FORMAT_LINEAR_16;
      break;
    case SF_FLOAT:
      NSclass(&sfh2) = SND_FORMAT_FLOAT;
      break;
    default:
      NSclass(&sfh2) = 0;
      break;
    }
    if (swap) {
      byte_reverse4(&NSchans(&sfh2));
      byte_reverse4(&NSmagic(&sfh2));
      byte_reverse4(&NSsrate(&sfh2));
      byte_reverse4(&NSdsize(&sfh2));
      byte_reverse4(&NSdloc(&sfh2));
      byte_reverse4(&NSclass(&sfh2));
    }

    /* Don't forget to swap the header data ! */
    if (swap) {
      byte_reverse4(&(sfh2.sfinfo.sf_magic));
      byte_reverse4(&(sfh2.sfinfo.sf_srate));
      byte_reverse4(&(sfh2.sfinfo.sf_chans));
      byte_reverse4(&(sfh2.sfinfo.sf_packmode)); 
    }
    
    if(wheader(sf2,&sfh2)) {
      printf("Can't seem to write header on file %s\n", point);
      perror("main");
      exit(-1);
    }
    
    if(fsync(sf2) < 0) printf("bad fsync\n");
  }
  
  close(sf1);
  close(sf2);
  
  printf("\ndone.\n");
  exit(0);
}
