#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sndlibsupport.h>

#define MAX_TIME_CHARS  64

static void usage(void);



/* ---------------------------------------------------------------- usage --- */
static void
usage()
{
   printf("usage:  \"sfprint [options] file [file...]\"\n");
   printf("        options:  -v verbose\n");
   printf("                  -q quiet (return status only)\n");
   exit(1);
}


/* ----------------------------------------------------------------- main --- */
/* Prints info about any number of sound files given as cmd-line arguments.
   If there's an error reading file, or if it's not a sound file with 
   recognizable header, prints error message to stderr, but continues
   processing argument list.  The info includes the file name, header type
   and data format, sampling rate, number of channels, "class" (number of
   bytes per sample word), duration, and maxamp stats (if any).  The format
   of much of this information intentionally looks like it always has in cmix.

   With the -v flag, also prints header size in bytes and number of sample
   frames in file.  With the -q flag, prints nothing.  This is intended for
   scripts that just want to know whether the file arguments are all sound
   files (and so only care about the return value).

   Returns 1 if any file could not be read or was not a sound file; 
   otherwise returns 0.
*/
int
main(int argc, char *argv[])
{
   int         i, fd, header_type, data_format, data_location;
   int         verbose = 0, quiet = 0, status = 0;
   int         nsamps, result, srate, nchans, class;
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
            case 'q':
               quiet = 1;
               break;
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
         if (!quiet)
            fprintf(stderr, "%s: %s\n", sfname, strerror(errno));
         status = 1;
         continue;
      }

      /* make sure it's a regular file or symbolic link */

      result = fstat(fd, &statbuf);
      if (result == -1) {
         if (!quiet)
            fprintf(stderr, "%s: %s\n", sfname, strerror(errno));
         status = 1;
         continue;
      }
      if (!S_ISREG(statbuf.st_mode) && !S_ISLNK(statbuf.st_mode)) {
         if (!quiet)
            fprintf(stderr, "%s is not a regular file or a link.\n", sfname);
         status = 1;
         continue;
      }
 
      /* read header and gather the data */

      result = sndlib_read_header(fd);
      if (result == -1) {
         if (!quiet)
            fprintf(stderr, "\nCan't read header of \"%s\"!\n\n", sfname);
         close(fd);
         status = 1;
         continue;
      }
      header_type = mus_header_type();
      if (NOT_A_SOUND_FILE(header_type)) {
         if (!quiet)
            fprintf(stderr, "\"%s\" is probably not a sound file\n", sfname);
         close(fd);
         status = 1;
         continue;
      }

      /* If we're just returning status, no need to compute print-out info. */
      if (quiet)
         continue;

      data_format = mus_header_format();
      data_location = mus_header_data_location();
      srate = mus_header_srate();
      nchans = mus_header_chans();
      class = mus_header_data_format_to_bytes_per_sample();
      nsamps = mus_header_samples();                 /* samples, not frames */
      dur = (float)(nsamps / nchans) / (float)srate;

      result = sndlib_get_current_header_comment(fd, &sfc);
      if (result == -1) {
         fprintf(stderr, "Can't read header comment!\n");
         sfc.offset = -1;          /* make sure it's ignored below */
         sfc.comment[0] = '\0';
         /* but keep going */
      }

      /* format maxamp timetag string, as in "Fri May 28 09:41:51 EDT 1999" */
      if (sfc.offset != -1)
         strftime(timestr, MAX_TIME_CHARS, "%a %b %d %H:%M:%S %Z %Y",
                                                    localtime(&sfc.timetag));

      /* print it out */

      printf(":::::::::::::::::::\n%s\n:::::::::::::::::::\n", sfname);
      printf("%s, %s\n", mus_header_type_name(header_type),
                         mus_data_format_name(data_format));
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
         if (!sfcomment_peakstats_current(&sfc, fd))
            printf("WARNING: maxamp stats not up-to-date -- run sndpeak.\n\n");
      }
      else {
         printf("(no maxamp stats)\n");
      }
      if (sfc.comment[0])
         printf("comment: %s\n", sfc.comment);
      printf("duration: %f seconds\n", dur);

      close(fd);
   }

   return status;
}

