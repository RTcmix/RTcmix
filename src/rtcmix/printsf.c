#include "../H/byte_routines.h"
#include "../H/sfheader.h"
#include "../H/ugens.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

static SFCODE	ampcode = {
	SF_MAXAMP,
	sizeof(SFMAXAMP) + sizeof(SFCODE)
}; 

extern int swap;

void printsf(SFHEADER *sfh)
{
  SFMAXAMP sfm;
  SFCODE *sizer;
  SFCOMMENT sfcm;
  char *cp;
  char date[26];
  int i;
  
  printf("sr: %f nchans: %d class: %d\n",
	 sfsrate(sfh),sfchans(sfh),sfclass(sfh));
  cp = getsfcode(sfh,SF_MAXAMP);
  if(cp != NULL) {
    bcopy(cp + sizeof(SFCODE), (char *) &sfm, sizeof(SFMAXAMP));
    
    if (swap) {
      for (i=0;i<sfchans(sfh);i++) {
	byte_reverse4(&(sfm).value[i]);
	byte_reverse4(&(sfm).samploc[i]);
      }
      byte_reverse4(&(sfm).timetag);
    }

    for(i=0; i<sfchans(sfh); i++)
      printf("channel %d: maxamp: %e loc: %d\n",
	     i,sfmaxamp(&sfm,i),sfmaxamploc(&sfm,i));
		if(sfmaxamptime(&sfm)) {
		  strcpy(date,ctime(&sfmaxamptime(&sfm)));
		  printf("date=%s\n",date);
		}
  }
  printf("\n");
  cp = getsfcode(sfh,SF_COMMENT);
  if(cp != NULL) {
    sizer = (SFCODE *) cp;
    if (swap) {
      byte_reverse2(&sizer->bsize);
      byte_reverse2(&sizer->code);
    }
    bcopy(cp + sizeof(SFCODE) , (char *) &sfcm, sizer->bsize);
    printf("Comment on soundfile: \n%s\n",&sfcomm(&sfcm,0));
  }
}
