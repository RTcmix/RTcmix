#ifdef USE_SNDLIB

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include "../H/sndlibsupport.h"

#define UNKNOWN_TYPE_MSG                          \
"%s is either a headerless sound file, an unknown \
type of sound file, or not a sound file at all.\n"

#define MAX_TIME_CHARS  64

int main(int, char **);
static time_t file_write_date(char *);


/* ------------------------------------------------------ file_write_date --- */
/* nabbed from sndlib's sound.c */
static time_t
file_write_date(char *filename)
{ 
   int         result;
   struct stat statbuf;

   result = stat(filename, &statbuf);
   if (result < 0)
      return result;
   return (statbuf.st_mtime);
} 


/* ----------------------------------------------------------------- main --- */
int
main(int argc, char *argv[])
{
   int         n, fd, header_type, srate, nchans, class, nsamps, result;
   float       dur;
   char        *sfname, timestr[MAX_TIME_CHARS];
   time_t      date;
   SFComment   sfc;
   struct stat statbuf;

   create_header_buffer();

   while (--argc) {
      sfname = *++argv;

      /* see if file exists and we can read it */

      fd = open(sfname, O_RDONLY);
      if (fd == -1) {
         fprintf(stderr, "%s: %s\n", sfname, strerror(errno));
         continue;
      }
      close(fd);

      /* make sure it's a regular file or symbolic link */

      result = stat(sfname, &statbuf);
      if (result == -1) {
         fprintf(stderr, "%s: %s\n", sfname, strerror(errno));
         continue;
      }
      if (!S_ISREG(statbuf.st_mode) && !S_ISLNK(statbuf.st_mode)) {
         fprintf(stderr, "%s is not a regular file or a link.\n", sfname);
         continue;
      }
 
      /* read header and gather the data */

      result = c_read_header(sfname);
      if (result == -1) {
         fprintf(stderr, "\nCan't read header of \"%s\"!\n\n", sfname);
         continue;
      }
      header_type = c_snd_header_type();
      if (header_type == unsupported_sound_file
                                           || header_type == raw_sound_file) {
         fprintf(stderr, UNKNOWN_TYPE_MSG, sfname);
         continue;
      }
      srate = c_snd_header_srate();
      nchans = c_snd_header_chans();
      class = c_snd_header_datum_size();           /* bytes per sample */
      nsamps = c_snd_header_data_size();           /* samples, not frames */
      dur = (float)(nsamps / nchans) / (float)srate;

      date = file_write_date(sfname);
      strftime(timestr, MAX_TIME_CHARS, "%a %d-%b-%y %H:%M %Z",
                                                         localtime(&date));
      result = sndlib_get_current_header_comment(sfname, &sfc);
      if (result == -1) {
         fprintf(stderr, "\nCan't read header comment of \"%s\"\n\n", sfname);
         continue;
      }

      /* print it out */

      printf(":::::::::::::::::::\n%s\n:::::::::::::::::::\n", sfname);
      printf("sr: %d nchans: %d class: 2\n", srate, nchans, class);
      if (sfc.offset >= 0) {
         for (n = 0; n < nchans; n++)
            printf("channel %d: maxamp: %g loc: %d\n",
                                           n, sfc.peak[n], sfc.peakloc[n]);
      }
      printf("date=%s\n\n", timestr);
      printf("Comment on soundfile: %s\n", sfc.comment);
      printf("Duration of file is %f seconds.\n", dur);
   }

   return 0;
}


/*****************************************************************************/
#else /* !USE_SNDLIB */
/*****************************************************************************/

#include "../H/sfheader.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#define  INT 2
static SFCODE	ampcode = {
	SF_MAXAMP,
	sizeof(SFMAXAMP) + sizeof(SFCODE)
}; 

int swap;

main(argc,argv)
     
     int argc;
     char **argv;
     
{
  int i,sf,result,headersize;
  struct stat sfst;
  float dur;
  SFHEADER sfh;
  char *sfname;
  
  while(--argc) {
    sfname = *++argv;
    readopensf(sfname,sf,sfh,sfst,"sfprint",result)
      if(result < 0) continue;
    printf(":::::::::::::::::::\n"); 
    printf("%s \n:::::::::::::::::::\n" ,sfname);
    printsf(&sfh);
    headersize = sizeof(SFHEADER);
    dur = (float)(sfst.st_size - headersize)
      /(float)sfclass(&sfh)/(float)sfchans(&sfh)
      /sfsrate(&sfh);
    printf("Duration of file is %f seconds.\n",dur);
    close(sf);
  }
}

#endif /* !USE_SNDLIB */

