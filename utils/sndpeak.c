#ifdef USE_SNDLIB

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <assert.h>
#include "../H/byte_routines.h"
#include "../H/sndlibsupport.h"

/* Revision of cmix sndpeak program with sndlib support and other improvements.
                                                          -- J. Gibson, 6/6/99
*/

/* #define NDEBUG */     /* define to disable asserts */

/* CAUTION: Don't uncomment this without fixing the code it enables.
   Problem is that rename won't move files across file systems.
   Solution would be to create tmp file in same dir as input file.
   But safer to just advise users anyway and not try to hide the
   file shuffling from them.
*/
/* #define AUTO_SHUFFLE */

#define TMP_SUFFIX  ".sndpeak"

#define HEADER_TYPE_SHUFFLE_MSG "\
"

#define HEADER_SIZE_SHUFFLE_MSG "\
Input file \"%s\" doesn't have enough room in header for peak stats.\n\
Sndpeak will copy input file to \"%s\"\n\
and write peak stats to that file instead.\n\
Replace the original when you're satisfied all is well.\n"

#define UNSUPPORTED_FORMAT_MSG "\"%s\" not in a supported data format.\n\
(Can be 16-bit linear or 32-bit float, either byte order.)\n"

#define NORM(x,y) ((float) (y) / (float) (x == SF_SHORT ? 32767.0 : 1.0))

static void usage(void);



/* ---------------------------------------------------------------- usage --- */
static void
usage()
{
   printf("usage:  \"sndpeak [options] file [file...]\"\n");
   printf("        options:  -n  don't update header, just print\n");
   printf("                  -q  quiet\n");
   printf("                  -v  verbose\n");
   exit(1);
}


/* ----------------------------------------------------------------- main --- */
/* Finds the peak amplitude (absolute value) of a soundfile and, if user
   wishes, updates the header peak stats accordingly.
*/

#define CONTINUE { exitcode = 1; continue; }

int
main(int argc, char *argv[])
{
   int         i, result, infd, outfd, type, format, indataloc, outdataloc;
   int         nchans, exitcode, update, verbose, quiet;
   long        startframe, nframes;
   long        peakloc[MAXCHANS];
   float       peak[MAXCHANS];
   char        *sfname, *outname;
   SFComment   sfc;
   struct stat statbuf;

   outfd = -1;
   outdataloc = startframe = exitcode = verbose = quiet = 0;
   update = 1;

   if (argc < 2)
      usage();

   for (i = 1; i < argc; i++) {
      char *arg = argv[i];

      if (arg[0] == '-') {
         switch (arg[1]) {
            case 'n':
               update = 0;
               break;
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
      infd = open(sfname, (update ? O_RDWR : O_RDONLY));
      if (infd == -1) {
         fprintf(stderr, "%s: %s\n", sfname, strerror(errno));
         CONTINUE;
      }

      /* make sure it's a regular file or symbolic link */
      result = fstat(infd, &statbuf);
      if (result == -1) {
         fprintf(stderr, "%s: %s\n", sfname, strerror(errno));
         close(infd);
         CONTINUE;
      }
      if (!S_ISREG(statbuf.st_mode) && !S_ISLNK(statbuf.st_mode)) {
         fprintf(stderr, "%s is not a regular file or a link.\n", sfname);
         close(infd);
         CONTINUE;
      }
 
      /* read header and gather info */
      if (sndlib_read_header(infd) == -1) {
         fprintf(stderr, "\nCan't read header of \"%s\"!\n\n", sfname);
         close(infd);
         CONTINUE;
      }

      type = c_snd_header_type();
      format = c_snd_header_format();

      if (NOT_A_SOUND_FILE(type) || INVALID_DATA_FORMAT(format)) {
         fprintf(stderr, "\"%s\" is probably not a sound file\n", sfname);
         close(infd);
         CONTINUE;
      }

      if (! SUPPORTED_DATA_FORMAT(format)) {
         fprintf(stderr, UNSUPPORTED_FORMAT_MSG, sfname);
         close(infd);
         CONTINUE;
      }
 
      if (sndlib_get_current_header_comment(infd, &sfc) == -1) {
         fprintf(stderr, "Can't read peak stats for \"%s\"\n", sfname);
         close(infd);
         CONTINUE;
      }

      if (verbose) {
         printf("Input: %s\n", sfname);
         printf(sndlib_print_current_header_stats(infd, &sfc, 2));
      }

      /* NOTE: must get these before tmp outfd becomes current header! */
      nchans = c_snd_header_chans();
      indataloc = c_snd_header_data_location();
      nframes = c_snd_header_data_size() / nchans;

      /* Make sure this is a header type sndlib can write, and that
         there's enough room in the header to store the peak stats comment.
       */
      if (update) {
         if (! WRITEABLE_HEADER_TYPE(type)) {
            fprintf(stderr, 
                    "%s: Can't update peak stats for this type of header.\n",
                                                                     sfname);
            close(infd);
            CONTINUE;
         }

         if (type == AIFF_sound_file)
            sndlib_current_header_match_aiff_flavor();

         /* If input file doesn't have a large enough space for the structured
            peak stats comment, write to a tmp file, and replace the input
            file with it when we're all done.
         */
         if (!sndlib_current_header_comment_alloc_good(NULL)) {
            int srate = c_snd_header_srate();
#ifdef AUTO_SHUFFLE
            outname = strdup(tmpnam(NULL));
            if (outname == NULL) {
               perror("sndpeak: strdup");
               exit(1);
            }
#else /* !AUTO_SHUFFLE */
            outname = (char *)calloc(FILENAME_MAX, sizeof(char));
            strncpy(outname, sfname, (FILENAME_MAX - strlen(TMP_SUFFIX)) - 1);
            strcat(outname, TMP_SUFFIX);
            printf(HEADER_SIZE_SHUFFLE_MSG, sfname, outname);
            result = stat(outname, &statbuf);
            if (result != -1) {
               fprintf(stderr, "Oops! \"%s\" already exists.\n", outname);
               CONTINUE;
            }
            if (errno != ENOENT) {
               fprintf(stderr, "Error creating output file (%s)\n",
                                                            strerror(errno));
               CONTINUE;
            }
#endif /* !AUTO_SHUFFLE */
            /* NOTE: must be rdwr, or won't be able to read header later! */
            outfd = open(outname, O_RDWR | O_CREAT | O_TRUNC, 0644);
            if (outfd == -1) {
               fprintf(stderr, "Can't create output file (%s)\n",
                                                            strerror(errno));
               exit(1);
            }
            if (sndlib_write_header(outfd, 0, type, format, srate, nchans,
                                                 NULL, &outdataloc) == -1) {
               fprintf(stderr, "Error writing output file header\n");
               exit(1);
            }
         }
      }

      if (sndlib_findpeak(infd, outfd, indataloc, outdataloc, format, nchans,
                                 startframe, nframes, peak, peakloc) == -1) {
         fprintf(stderr, "Error while scanning \"%s\"\n", sfname);
         close(infd);
         CONTINUE;
      }

      /* Write the peak stats and comment. */
      if (update) {
         long len;
         int  loc, tmpfd;

         if (outfd == -1) {
            tmpfd = infd;
            loc = indataloc;
         }
         else {
            tmpfd = outfd;
            loc = outdataloc;
         }

         if (sndlib_put_header_comment(tmpfd, peak, peakloc,
                                                       sfc.comment) == -1) {
            fprintf(stderr, "Can't write peak stats for \"%s\"\n", sfname);
            exit(1);
         }

         /* update header for bytes of sound data written (even for infd) */
         len = lseek(tmpfd, 0, SEEK_END);
         if (len == -1) {
            perror("sndpeak: lseek");
            exit(1);
         }
         if (sndlib_set_header_data_size(tmpfd, type, len - loc) == -1) {
            fprintf(stderr, "Can't update header data size for \"%s\"\n",
                                                                   sfname);
            /* but keep going */
         }

         close(infd);

         if (outfd != -1) {
#ifdef AUTO_SHUFFLE
            char *tmp;
#endif

            if (close(outfd) == -1) {
               perror("sndpeak: close");
               exit(1);
            }

#ifdef AUTO_SHUFFLE
            /* mv -f tmpoutfile inputfile, but with extra caution */
            tmp = tmpnam(NULL);
            if (rename(sfname, tmp) == -1) {
               perror("sndpeak: rename");
               exit(1);
            }
            if (rename(outname, sfname) == -1) {
               perror("sndpeak: rename");
               exit(1);
            }
            if (unlink(tmp) == -1) {
               perror("sndpeak: unlink");
               exit(1);
            }
#else /* !AUTO_SHUFFLE */
#endif /* !AUTO_SHUFFLE */
         }
      }

      if (!quiet) {
         int n, ds = c_snd_datum_size(format);

         printf("Peak stats for file \"%s\":\n", sfname);
         for (n = 0; n < nchans; n++) {
            printf("  channel %d:  ", n);
            if (peak[n])
               printf("%f (absolute %f) at frame %ld\n",
                      NORM(ds, peak[n]), peak[n], peakloc[n]);
            else
               printf("silence\n");
         }
      }
   }

   exit(exitcode);
}


#endif /* !USE_SNDLIB */

