/* check_byte_order.c -- attempts to read byte-swapped Hybrid header */

/* Hacked by DT 1/27/98 for Linux Intel */
/* Only for use with BSD/IRCAM snd files */

#include "../H/sfheader.h"
#include "../H/byte_routines.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

int swap;
short isNext;

int check_byte_order(SFHEADER *sfh,char *prog,char *sfname)
{
  int retval;
 
  /* return 0 if file is ok (native or can be swapped) */
  /* return 1 otherwise */

  /* Check the soundfile header */
  if (ismagic(sfh)) {
    fprintf(stdout,"This is a BSD/Next/IRCAM hybrid soundfile\n");
    fprintf(stdout,"Soundfile has same endian-ness ... no need to swap\n");
    swap = 0;
    retval = 0;
    isNext = 0;
  }
  else if (is_swapmagic(sfh)) {
    fprintf(stdout,"This is a BSD/Next/IRCAM hybrid soundfile\n");
    fprintf(stdout,"Soundfile has swapped endian-ness ...need to swap\n");
    swap = 1;
    retval = 0;
    isNext = 0;
  }
  else if (is_nsfmagic(sfh)) {
    fprintf(stdout,"This is a Next soundfile\n");
    fprintf(stdout,"Soundfile has same endian-ness ... no need to swap\n");
    swap = 0;
    retval = 0;
    isNext = 1;
  }
  else if (is_nsfswapmagic(sfh)) {
    fprintf(stdout,"This is a Next soundfile\n");
    fprintf(stdout,"Soundfile has swapped endian-ness ... need to swap\n");
    swap = 1;
    retval = 0;
    isNext = 1;
  }
  else {
    retval = 1;
    return (retval);
  }

  /* If it's a Next format soundfile we need to copy in header information */
  if (isNext) {
    printf("Copying Next header information into IRCAM header\n");
    (sfh)->sfinfo.sf_chans = NSchans(sfh);
    (sfh)->sfinfo.sf_magic = SF_MAGIC;
    (sfh)->sfinfo.sf_srate = (int) NSsrate(sfh);
    if (swap) {
      byte_reverse4(&NSclass(sfh));
    }
    switch (NSclass(sfh)) {
    case SND_FORMAT_LINEAR_16:
      (sfh)->sfinfo.sf_packmode = SF_SHORT;
      break;
    case SND_FORMAT_FLOAT:
      (sfh)->sfinfo.sf_packmode = SF_FLOAT;
      break;
    default:
      (sfh)->sfinfo.sf_packmode = 0;
      break;
    }
    (sfh)->sfinfo.sf_codes = 0;
  }

  /* If it's a reversed-endian soundfile, we need to swap information */
  if (swap) {
    (sfh)->sfinfo.sf_magic = reverse_int4(&(sfh)->sfinfo.sf_magic);

    if (isNext)
      (sfh)->sfinfo.sf_srate = reverse_int4(&NSsrate(sfh));
    if (!isNext)
      byte_reverse4(&(sfh)->sfinfo.sf_srate);

    (sfh)->sfinfo.sf_chans = reverse_int4(&(sfh)->sfinfo.sf_chans);

    if (!isNext)
      (sfh)->sfinfo.sf_packmode = reverse_int4(&(sfh)->sfinfo.sf_packmode); 
  }

  return(retval);
}

