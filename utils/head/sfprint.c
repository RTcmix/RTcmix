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

#define MAX_TIME_CHARS  64

static void usage(void);



/* ---------------------------------------------------------------- usage --- */
static void
usage()
{
   printf("usage:  \"sfprint [options] file [file...]\"\n");
   printf("        options:  -v verbose\n");
   exit(1);
}


/* ----------------------------------------------------------------- main --- */
int
main(int argc, char *argv[])
{
   int         i, fd, header_type, data_format, data_location, verbose = 0;
   int         nsamps, result, is_aifc, srate, nchans, class;
   float       dur;
   char        *sfname, timestr[MAX_TIME_CHARS];
   SFComment   sfc;
   struct stat statbuf;

   if (argc < 2)
      usage();

   for (i = 1; i < argc; i++) {
      char *arg = argv[i];

      if (arg[0] == '-') {
         switch (arg[1]) {
            case 'v':
               verbose = 1;
               break;
            default:
               usage();
         }
         continue;
      }
      sfname = arg;

      /* see if file exists and we can read it */

      fd = open(sfname, O_RDONLY);
      if (fd == -1) {
         fprintf(stderr, "%s: %s\n", sfname, strerror(errno));
         continue;
      }

      /* make sure it's a regular file or symbolic link */

      result = fstat(fd, &statbuf);
      if (result == -1) {
         fprintf(stderr, "%s: %s\n", sfname, strerror(errno));
         continue;
      }
      if (!S_ISREG(statbuf.st_mode) && !S_ISLNK(statbuf.st_mode)) {
         fprintf(stderr, "%s is not a regular file or a link.\n", sfname);
         continue;
      }
 
      /* read header and gather the data */

      result = sndlib_read_header(fd);
      if (result == -1) {
         fprintf(stderr, "\nCan't read header of \"%s\"!\n\n", sfname);
         close(fd);
         continue;
      }
      header_type = c_snd_header_type();
      if (NOT_A_SOUND_FILE(header_type)) {
         fprintf(stderr, "\"%s\" is probably not a sound file\n", sfname);
         close(fd);
         continue;
      }
      is_aifc = sndlib_current_header_is_aifc();
      data_format = c_snd_header_format();
      data_location = c_snd_header_data_location();
      srate = c_snd_header_srate();
      nchans = c_snd_header_chans();
      class = c_snd_header_datum_size();           /* bytes per sample */
      nsamps = c_snd_header_data_size();           /* samples, not frames */
      dur = (float)(nsamps / nchans) / (float)srate;

      result = sndlib_get_current_header_comment(fd, &sfc);
      if (result == -1) {
         fprintf(stderr, "Can't read header comment!\n");
         sfc.offset = -1;          /* make sure it's ignored below */
         sfc.comment[0] = '\0';
         /* but keep going */
      }

      close(fd);                                   /* done with this now */

      /* format maxamp timetag string, as in "Fri May 28 09:41:51 EDT 1999" */
      if (sfc.offset != -1)
         strftime(timestr, MAX_TIME_CHARS, "%a %b %d %H:%M:%S %Z %Y",
                                                    localtime(&sfc.timetag));

      /* print it out */

      printf(":::::::::::::::::::\n%s\n:::::::::::::::::::\n", sfname);
      printf("%s, %s\n", is_aifc? "AIFC" : sound_type_name(header_type),
                                           sound_format_name(data_format));
      printf("sr: %d  nchans: %d  class: %d\n", srate, nchans, class);
      if (verbose)
         printf("frames: %d\nheader size: %d bytes\n",
                                             nsamps / nchans, data_location);
      if (sfc.offset != -1) {
         int n;
         for (n = 0; n < nchans; n++)
            printf("channel %d:  maxamp: %g  loc: %ld\n",
                                           n, sfc.peak[n], sfc.peakloc[n]);
         printf("maxamp updated: %s\n", timestr);
         if (statbuf.st_mtime > sfc.timetag + 2)
            printf("WARNING: maxamp stats not up-to-date -- run sndpeak.\n\n");
      }
      else {
         printf("(no maxamp stats)\n");
      }
      if (sfc.comment[0])
         printf("comment: %s\n", sfc.comment);
      printf("duration: %f seconds\n", dur);

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

