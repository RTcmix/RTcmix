#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <byte_routines.h>
#include <sndlibsupport.h>

/* Revision of cmix rescale program with sndlib support and other improvements.
   The drescale program has been folded into this one.
                                                         -- J. Gibson, 6/5/99
*/

/* This turns on verification of input file peak. Comment out for more speed. */
#define INPUT_PEAK_CHECK

#define BUFSIZE  32768

#ifndef DBL_EPSILON
   /* Difference between 1.0 and the minimum double greater than 1.0 */
   #define DBL_EPSILON 2.2204460492503131e-16
#endif

#define ABS(x)    ((x) < 0 ? (-x) : (x))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

char *progname = NULL;

static void usage(void);
static float drandom(long *);


/* ------------------------------------ usage, error and warning messages --- */

#define USAGE_MSG                                     " \n\
usage:  %s [options] input_filename [output_filename]   \n\
                                                                        \n\
  option   description                      default                     \n\
  --------------------------------------------------------------------- \n\
  -P NUM   desired peak amplitude           [32767]                     \n\
  -p NUM   peak amplitude of input file     [taken from file header]    \n\
  -f NUM   rescale factor                   [desired peak / input peak] \n\
  -r       overwrite INPUT file             [no]                        \n\
  -t       use dithering algorithm          [no]                        \n\
  -s NUM   input file skip (seconds)        [0.0]                       \n\
  -o NUM   output file skip (seconds)       [0.0]                       \n\
  -d NUM   duration of rescale (seconds)    [entire file]               \n\
  -e NUM   silence at end of file (seconds) [0.0]                       \n\
                                                                        \n\
  (Defaults take effect unless overridden by supplied values.)          \n\
                                                                        \n\
  (1) If no output file name, and no overwrite option (-r), output      \n\
      file will have same name as input file, but with \".rescale\".    \n\
  (2) Desired peak (-P) ignored if you also specify rescale factor (-f).\n\
"

#define NO_PEAK_AMP_MSG                                         " \n\
Can't calculate rescale factor without an input peak amplitude.   \n\
Do one of these things:                                           \n\
  - specify the factor yourself (e.g., -f 1.2)                    \n\
  - put a peak amplitude in the file header (e.g., with sndpeak)  \n\
  - tell %s what the file's peak amp is (use -p option). \
\n"

#define DANGEROUS_OVERWRITE_MSG                                 " \n\
Can't overwrite this input file, because the output samples would \n\
overtake the input samples. Run without the -r option.            \n\
\n"

#define STALE_PEAK_STATS_WARNING "\
WARNING: Calculating rescale factor from peak in file header, \n\
         but this peak info might not be up-to-date. \
\n"

#define BAD_PEAK_LOC_WARNING "\
WARNING: Peak stats in the input header might not be up-to-date,    \n\
         so peak locations in the output header will be unreliable. \n\
         Run sndpeak later if you care about this. \
\n"

#define OVERWRITE_CLICK_WARNING "\
WARNING: Since the input header is longer than the output header, \n\
         and you set an output skip, you may get a click at the   \n\
         beginning of the output file. \
\n"



/* ---------------------------------------------------------------- usage --- */
static void
usage()
{
   printf(USAGE_MSG, progname);
   exit(1);
}


/* -------------------------------------------------------------- drandom --- */
static float
drandom(long *seeds)
{
   int i1 = ((*seeds = *seeds * 1103515245 + 12345)>>16) & 077777;
   int i2 = ((*(seeds+1) = *(seeds+1) * 1103515245 + 12345)>>16) & 077777;
   return ((float)i1/32768. + (float)i2/32768. - 1.0);
}


/* ----------------------------------------------------------------- main --- */
int
main(int argc, char *argv[])
{
   int         i, replace, result, inswap, outswap;
   int         infd, outfd, intype, outtype, inheadersize, outheadersize;
   int         inclass, outclass, nchans, informat, outformat, srate, nsamps;
   int         limiter, dither, inpeak_uptodate;
   int         nbytes, inbytes, outbytes, durbytes, readbytes, bufcount;
   float       inpeak, specified_peak, desired_peak, actual_peak;
   double      factor, inskip, outskip, dur, empty;
   long        inskipbytes, outskipbytes, len, seeds[2];
   short       outbuf[BUFSIZE];
   float       inbuf[BUFSIZE];
   short       *bufp;
   char        *insfname, *outsfname;
   SFComment   insfc, outsfc;
   struct stat statbuf;

   /* get name of this program */
   progname = strrchr(argv[0], '/');
   if (progname == NULL)
      progname = argv[0];
   else
      progname++;

   if (argc == 1)
      usage();

   insfname = outsfname = NULL;
   replace = dither = inpeak_uptodate = bufcount = 0;
   inskip = outskip = dur = empty = 0.0;
   factor = inpeak = specified_peak = desired_peak = 0.0;

   for (i = 1; i < argc; i++) {
      char *arg = argv[i];

      if (arg[0] == '-') {
         switch (arg[1]) {
            case 'P':
               if (++i >= argc)
                  usage();
               desired_peak = (float)atof(argv[i]);
               break;
            case 'p':
               if (++i >= argc)
                  usage();
               specified_peak = (float)atof(argv[i]);
               break;
            case 'f':
               if (++i >= argc)
                  usage();
               factor = atof(argv[i]);
               break;
            case 'r':
               replace = 1;
               break;
            case 't':
               dither = 1;
               break;
            case 's':
               if (++i >= argc)
                  usage();
               inskip = atof(argv[i]);
               if (inskip < 0.0) {
                  fprintf(stderr, "Input file skip must be >= 0.\n");
                  exit(1);
               }
               break;
            case 'o':
               if (++i >= argc)
                  usage();
               outskip = atof(argv[i]);
               if (outskip < 0.0) {
                  fprintf(stderr, "Output file skip must be >= 0.\n");
                  exit(1);
               }
               break;
            case 'd':
               if (++i >= argc)
                  usage();
               dur = atof(argv[i]);
               if (dur <= 0.0) {
                  fprintf(stderr, "Duration must be greater than zero.\n");
                  exit(1);
               }
               break;
            case 'e':
               if (++i >= argc)
                  usage();
               empty = atof(argv[i]);
               if (empty < 0.0) {
                  fprintf(stderr, "Silence at end must be >= 0.\n");
                  exit(1);
               }
               break;
            default:  
               usage();
         }
      }
      else {
         if (insfname == NULL)
            insfname = arg;
         else if (outsfname == NULL)
            outsfname = arg;
         else
            usage();
      }
   }

   if (insfname == NULL) {
      fprintf(stderr, "You haven't specified an input file.\n");
      exit(1);
   }

   /* Print out the specified options. */

   if (replace)
      printf("Writing over input file.\n");
   if (inskip > 0.0)
      printf("Input skip = %f\n", inskip);
   if (outskip > 0.0)
      printf("Output skip = %f\n", outskip);
   if (dur > 0.0)
      printf("Rescale duration = %f\n", dur);
   if (empty > 0.0)
      printf("Writing %g seconds of silence at end.\n", empty);
   if (factor != 0.0)
      printf("Specified rescale factor = %f\n", factor);
   if (specified_peak > 0.0)
      printf("Specified peak of input file = %f\n", specified_peak);
   if (desired_peak > 0.0) {
      if (factor > 0.0) {
         printf("You specified both a factor and a desired peak ...");
         printf(" ignoring your desired peak.\n");
      }
      else {
         if (desired_peak > 32767.0) {
            printf("Desired peak is out of range ... clamping to 32767.\n");
            desired_peak = 32767.0;
         }
         else
            printf("Desired peak = %f\n", desired_peak);
      }
   }
   else
      desired_peak = 32767.0;
   if (dither)
      printf("Dithering algorithm requested.\n");


   /************************************************************************/
   /* SET UP FILES AND PARAMETERS                                          */
   /************************************************************************/

   /*** Input File *********************************************************/

   infd = open(insfname, O_RDONLY);
   if (infd == -1) {
      perror(progname);
      exit(1);
   }
   if (fstat(infd, &statbuf) == -1) {
      perror(progname);
      exit(1);
   }
   if (!S_ISREG(statbuf.st_mode) && !S_ISLNK(statbuf.st_mode)) {
      fprintf(stderr, "%s is not a regular file or a link.\n", insfname);
      exit(1);
   }

   result = sndlib_read_header(infd);
   if (result == -1) {
      fprintf(stderr, "Can't read header of %s (%s)\n",
                                                  insfname, strerror(errno));
      exit(1);
   }

   intype = mus_header_type();
   informat = mus_header_format();
   nchans = mus_header_chans();
   srate = mus_header_srate();
   inheadersize = mus_header_data_location();
   inclass = mus_header_data_format_to_bytes_per_sample();

   if (NOT_A_SOUND_FILE(intype) || INVALID_DATA_FORMAT(informat)) {
      fprintf(stderr, "\"%s\" probably not a sound file.\n", insfname);
      exit(1);
   }

   if (sndlib_get_current_header_comment(infd, &insfc) == -1) {
      fprintf(stderr, "Can't read peak stats for input file\n");
      exit(1);
   }

   printf("Input: %s\n", insfname);
   printf(sndlib_print_current_header_stats(infd, &insfc, 2));

   if (!IS_FLOAT_FORMAT(informat))
      printf("NOTE: Input file is not floating point.\n");

   /* Store overall peak, in case we need it later. */
   if (SFCOMMENT_PEAKSTATS_VALID(&insfc)) {
      for (i = 0, inpeak = 0.0; i < nchans; i++) 
         if (insfc.peak[i] > inpeak)
            inpeak = insfc.peak[i];

      /* See if peak stats are up-to-date (i.e., file not modified after
         peak stats timetag).
      */
      inpeak_uptodate = sfcomment_peakstats_current(&insfc, infd);
   }
   if (!inpeak_uptodate)
      printf(BAD_PEAK_LOC_WARNING);   /* output header peak locs unreliable */

   /* Limiter turned on ... unless the rescale factor (whether computed or
      specified by user) multiplied by the input peak is not greater than
      32767, AND we're using up-to-date header peak stats or the input file
      has shorts. (See below.)  ... whew!
   */
   limiter = 1;

   /* If user doesn't give a rescale factor, we need the file (or specified)
      peak to calculate the factor.
   */
   if (factor == 0.0) {
      if (specified_peak) {                  /* use instead of header peak */
         if (inclass == SF_SHORT && specified_peak > 32767) {
            printf("Specified peak exceeds range of input file");
            printf(" ... resetting to 32767.\n");
            specified_peak = 32767.0;
         }
         inpeak = specified_peak;

         /* need sfm for calculating output peak stats at end */
         for (i = 0; i < nchans; i++)
            insfc.peak[i] = specified_peak;
      }
      else {                                 /* use peak from header */
         if (inpeak_uptodate)
            limiter = 0;
         else {
            if (inpeak == 0.0) {
               fprintf(stderr, NO_PEAK_AMP_MSG, progname);
               exit(1);
            }
            printf(STALE_PEAK_STATS_WARNING);
         }
         printf("Peak amplitude of input file is %f.\n", inpeak);
      }
      factor = ((double)desired_peak / (double)inpeak) + DBL_EPSILON;
      printf("Computed rescale factor = %f\n", factor);
   }
   else {
      if (inpeak_uptodate && factor * inpeak <= 32767.0)
         limiter = 0;
   }

   if (inclass == SF_SHORT && factor <= 1.0)
      limiter = 0;

   if (limiter)
      printf("Might get samples out of range ... turning on limiter.\n");


   /*** Output File ********************************************************/

   outclass = SF_SHORT;           /* we always rescale to shorts */

   outformat = IS_BIG_ENDIAN_FORMAT(informat) ?  MUS_BSHORT : MUS_LSHORT;

   /* If input header is a type that sndlib can't write, output to AIFF. */
   if (WRITEABLE_HEADER_TYPE(intype))
      outtype = intype;
   else {
      outtype = MUS_AIFC;
      outformat = MUS_BSHORT;
   }
   if (outtype == MUS_AIFC)
      mus_header_set_aifc(0);                    /* we want AIFF, not AIFC */

   if (replace) {                 /* will open 2 descriptors for same file */
      int      fd;
      FILE     *stream;

      /* Only way to know for sure if headersize will change is to write
         a temp file with the header that would go on the input file when
         the rescale is finished. (Note that even with the same header
         type, the input file may not have the same comment allocation
         that sndlib-savvy cmix maintains.)
      */
      stream = tmpfile();
      fd = fileno(stream);
      if (fd == -1) {
         fprintf(stderr, "Trouble preparing output header\n");
         exit(1);
      }
      if (sndlib_write_header(fd, 0, outtype, outformat, srate, nchans,
                                               NULL, &outheadersize) == -1) {
         fprintf(stderr, "Trouble preparing output header\n");
         exit(1);
      }
      close(fd);
      fclose(stream);  /* deleted automatically, since created with tmpfile */

      outsfname = insfname;
   }
   else {                         /* need to create a new short int file */
      int   fd;

      if (outsfname == NULL) {    /* no filename specified */
         long  size;
         char  suffix[] = ".rescale";

         /* Build and allocate output file name. */
         size = strlen(insfname) + strlen(suffix) + 1;
         if (size > FILENAME_MAX) {
            fprintf(stderr, "Output file name is too long!\n");
            exit(1);
         }
         outsfname = (char *)malloc(size);
         if (!outsfname) {
            perror("Can't malloc output file name");
            exit(1);
         }
         strcpy(outsfname, insfname);
         strcat(outsfname, suffix);
      }

      /* Check that file doesn't exist. */
      result = stat(outsfname, &statbuf);
      if (result == 0 || errno != ENOENT) {
         fprintf(stderr, "\"%s\" already exists.\n", outsfname);
         exit(1);
      }

      /* Create file; prepare and write header. */

      fd = open(outsfname, O_RDWR | O_CREAT | O_TRUNC, 0644);
      if (fd == -1) {
         fprintf(stderr, "Can't create file %s (%s)\n",
                                                  outsfname, strerror(errno));
         exit(1);
      }

      if (sndlib_write_header(fd, 0, outtype, outformat, srate, nchans,
                                               NULL, &outheadersize) == -1) {
         fprintf(stderr, "Trouble writing output header\n");
         exit(1);
      }
      close(fd);                                 /* will reopen just below */
   }

   /* Open a file descriptor for the output file name (which might be the
      same as the input file name).
   */
   outfd = open(outsfname, O_RDWR);
   if (outfd == -1) {
      perror(progname);
      exit(1);
   }

   printf("Writing output to \"%s\"\n", outsfname);


   /*** Seek ***************************************************************/

   inskipbytes = (long)(inskip * srate * nchans * inclass);
   outskipbytes = (long)(outskip * srate * nchans * outclass);

   if (replace) {
      long outlead = (outskipbytes - inskipbytes)
                                           + (outheadersize - inheadersize);
      if ((BUFSIZE * outclass) + outlead > BUFSIZE * inclass) {
         fprintf(stderr, DANGEROUS_OVERWRITE_MSG);
         exit(1);
      }
      if (outskip > 0.0 && inheadersize > outheadersize)
         printf(OVERWRITE_CLICK_WARNING);
   }

   /* make sure it lands on sample block */
   inskipbytes -= inskipbytes % (inclass * nchans);
   if (lseek(infd, inskipbytes + inheadersize, SEEK_SET) == -1) {
      fprintf(stderr, "Bad skip on input file!\n");
      exit(1);
   }

   /* make sure it lands on sample block */
   outskipbytes -= outskipbytes % (outclass * nchans);
   if (lseek(outfd, outskipbytes + outheadersize, SEEK_SET) == -1) {
      fprintf(stderr, "Bad skip on output file!\n");
      exit(1);
   }

#if MUS_LITTLE_ENDIAN
   inswap = IS_BIG_ENDIAN_FORMAT(informat);
   outswap = IS_BIG_ENDIAN_FORMAT(outformat);
#else
   inswap = IS_LITTLE_ENDIAN_FORMAT(informat);
   outswap = IS_LITTLE_ENDIAN_FORMAT(outformat);
#endif


   /************************************************************************/
   /* RESCALE LOOP                                                         */
   /************************************************************************/

   printf("Rescaling....\n");
   fflush(stdout);

#ifdef INPUT_PEAK_CHECK
   actual_peak = inpeak;
#endif

   readbytes = inbytes = BUFSIZE * inclass;
   durbytes = dur * inclass * nchans * srate;

   bufp = (short *)inbuf;

   seeds[0] = 17;      /* seeds for dither */
   seeds[1] = 31;

   while (1) {
      double dblsamp;

      if (dur) {
         if (durbytes <= readbytes)
            inbytes = durbytes;
         durbytes -= inbytes;
      }

      nbytes = read(infd, (char *)inbuf, inbytes);
      if (nbytes == -1) {
         fprintf(stderr, "Read on input file failed (%s)\n", strerror(errno));
         close(infd);
         close(outfd);
         exit(1);
      }
      if (nbytes == 0)                  /* reached EOF -- time to stop */
         break;

      nsamps = nbytes / inclass;
      outbytes = nsamps * outclass;

      if (inswap) {
         if (inclass == SF_FLOAT) {
            for (i = 0; i < nsamps; i++)
               byte_reverse4(&inbuf[i]);
         }
         else {
            for (i = 0; i < nsamps; i++)
               byte_reverse2(&bufp[i]);
         }
      }

      if (limiter) {
         int  samp, clipmax, numclipped;

         clipmax = numclipped = 0;

         if (inclass == SF_FLOAT) {
            for (i = 0; i < nsamps; i++) {
#ifdef INPUT_PEAK_CHECK
               float fabsamp = fabs(inbuf[i]);
               if (fabsamp > actual_peak)
                  actual_peak = fabsamp;
#endif
               /* NB: Assigning to dblsamp seems to be necessary for accuracy.
                  "samp = (int)((double)inbuf[i] * factor)" was NOT always.
               */
               dblsamp = (double)inbuf[i] * factor;
               if (dither)
                  dblsamp += (double)drandom(seeds);
               samp = (int)dblsamp;

               if (samp < -32767) {             /* not -32768 */
                  if (samp < -clipmax)
                     clipmax = -samp;
                  samp = -32767;
                  numclipped++;
               }
               else if (samp > 32767) {
                  if (samp > clipmax)
                     clipmax = samp;
                  samp = 32767;
                  numclipped++;
               }
               outbuf[i] = (short)samp;
            }
         }
         else {                                 /* SF_SHORT */
            for (i = 0; i < nsamps; i++) {
#ifdef INPUT_PEAK_CHECK
               int absamp = ABS(bufp[i]);
               if (absamp > (int)actual_peak)
                  actual_peak = (float)absamp;
#endif
               dblsamp = (double)bufp[i] * factor;
               if (dither)
                  dblsamp += (double)drandom(seeds);
               samp = (int)dblsamp;

               if (samp < -32767) {             /* not -32768 */
                  if (samp < -clipmax)
                     clipmax = -samp;
                  samp = -32767;
                  numclipped++;
               }
               else if (samp > 32767) {
                  if (samp > clipmax)
                     clipmax = samp;
                  samp = 32767;
                  numclipped++;
               }
               outbuf[i] = (short)samp;
            }
         }
         bufcount++;
         if (numclipped) {
            float loc1 = (float)(((bufcount - 1) * BUFSIZE) / nchans)
                                                                / (float)srate;
            float loc2 = (float)((bufcount * BUFSIZE) / nchans) / (float)srate;
            if (outskip > 0.0) {
               loc1 += outskip;
               loc2 += outskip;
            }
            printf("  CLIPPING: %4d samps, max: %d, output time: %f - %f\n",
                   numclipped, clipmax, loc1, loc2);
         }
      }

      else {                                   /* !limiter */
         if (inclass == SF_FLOAT) {
            for (i = 0; i < nsamps; i++) {
#ifdef INPUT_PEAK_CHECK
               float fabsamp = fabs(inbuf[i]);
               if (fabsamp > actual_peak)
                  actual_peak = fabsamp;
#endif
               dblsamp = (double)inbuf[i] * factor;
               if (dither)
                  dblsamp += (double)drandom(seeds);
               outbuf[i] = (short)dblsamp;
            }
         }
         else {                                /* SF_SHORT */
            for (i = 0; i < nsamps; i++) {
#ifdef INPUT_PEAK_CHECK
               int absamp = ABS(bufp[i]);
               if (absamp > (int)actual_peak)
                  actual_peak = (float)absamp;
#endif
               dblsamp = (double)bufp[i] * factor;
               if (dither)
                  dblsamp += (double)drandom(seeds);
               outbuf[i] = (short)dblsamp;
            }
         }
      }

      if (outswap) {
         for (i = 0; i < nsamps; i++)
            byte_reverse2(&outbuf[i]);
      }

      nbytes = write(outfd, (char *)outbuf, outbytes);
      if (nbytes != outbytes) {
         fprintf(stderr, "Write on output file failed  %s\n",
                                      (nbytes == -1) ? strerror(errno) : "");
         close(infd);
         close(outfd);
         exit(1);
      }
   }


   /************************************************************************/
   /* CLEANUP                                                              */
   /************************************************************************/

   close(infd);

   if (empty > 0.0) {                   /* write empty buffers */
      int bytesleft, bufbytes;

      for (i = 0; i < BUFSIZE; i++)
         outbuf[i] = 0;

      bufbytes = BUFSIZE * outclass;
      bytesleft = empty * srate * nchans * outclass;
      outbytes = MIN(bufbytes, bytesleft);

      for ( ; bytesleft > 0; bytesleft -= bufbytes) {
         if (bytesleft < outbytes)
            outbytes = bytesleft;
         if (write(outfd, (char *)outbuf, outbytes) != outbytes) {
            fprintf(stderr, "Bad write on output file!\n");
            close(outfd);
            exit(1);
         }
      }
   }

   if (replace) {
      long pos = lseek(outfd, 0L, SEEK_CUR);
      if (ftruncate(outfd, pos) < 0) 
         fprintf(stderr, "Bad truncation!\n");   /* but keep going */
   }

   for (i = 0; i < nchans; i++) {
      double opk = (double)insfc.peak[i];
#ifdef INPUT_PEAK_CHECK
      if (actual_peak != inpeak)
         opk += (double)(actual_peak - inpeak);
#endif
      opk *= factor;
      outsfc.peak[i] = (short)MIN(opk, 32767.0);
      outsfc.peakloc[i] = insfc.peakloc[i];
   }

   if (replace) {                             /* replace the input header */
      if (sndlib_write_header(outfd, 0, outtype, outformat, srate, nchans,
                                               NULL, &outheadersize) == -1) {
         fprintf(stderr, "Can't write header on \"%s\"\n", outsfname);
         close(outfd);
         exit(1);
      }
   }

   /* Make a text comment giving command line that wrote this file. */
   outsfc.comment[0] = '\0';
   nbytes = MAX_COMMENT_CHARS - 1;
   for (i = 0; i < argc && nbytes > 1; i++) {
      strncat(outsfc.comment, argv[i], nbytes - 1);
      strcat(outsfc.comment, " ");
      nbytes -= strlen(argv[i]);
   }
   outsfc.comment[MAX_COMMENT_CHARS - 1] = '\0';

   /* Write the peak stats and comment. */
   result = sndlib_put_header_comment(outfd, outsfc.peak, outsfc.peakloc,
                                                             outsfc.comment);
   if (result == -1) {
      fprintf(stderr, "Can't write peak stats for \"%s\"\n", outsfname);
      close(outfd);
      exit(1);
   }

   /* Update header for bytes of sound data written. */
   len = lseek(outfd, 0, SEEK_END);
   if (len == -1) {
      perror(progname);
      exit(1);
   }
   if (sndlib_set_header_data_size(outfd, outtype,
                                              len - outheadersize) == -1) {
      fprintf(stderr, "Can't update header data size for \"%s\"\n", outsfname);
      exit(1);
   }

   if (close(outfd) == -1) {
      fprintf(stderr, "Error closing output file (%s)\n", strerror(errno));
      exit(1);
   }

   printf("...done\n");

#ifdef INPUT_PEAK_CHECK
   if (actual_peak != inpeak)
      printf("Actual input peak different from expected: %f\n", actual_peak);
#endif

   return 0;
}

