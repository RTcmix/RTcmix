/*
 * FILE: resample.c
 * Sampling Rate Conversion Program
 */

#define RESAMPLE_VERSION (char *) "\
resample version 1.2 (jos at ccrma dot stanford dot edu)\n\
(sndlib support by johgibso at indiana dot edu)\n"

#include "filterkit.h"
#include "resample.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h>
#include <sndlibsupport.h>

#define OUT_TYPE    MUS_AIFF
//#define OUT_TYPE    MUS_RIFF
//#define OUT_TYPE    MUS_NEXT
//#define OUT_TYPE    MUS_IRCAM

#define OUT_FORMAT  MUS_BSHORT
//#define OUT_FORMAT  MUS_LSHORT  /* use this for RIFF only */

#define OUT_SUFFIX  ".resamp"

#define DEFAULT_ROLLOFF   0.9
#define DEFAULT_BETA      9.0
#define DEFAULT_LENGTH    65     /* NOTE: depends on being MAXNWING >= 8192 */

#define USAGE "\
  Two kinds of sampling rate conversion:                             \n\
     (1) using Kaiser-windowed low-pass filter (better)              \n\
     (2) using linear interpolation only, no filter (faster)         \n\
  For (1), use either no option, in which case you get the default,  \n\
  decent-quality resampling filter; use the -a option for a better   \n\
  quality filter; or use a combination of the -f, -b and -l options  \n\
  to design your own filter. (Read \"resample.doc\" to find out how.)\n\
  For (2), use -i.                                                   \n\
                                                                     \n\
  Usage: resample -r NEW_SRATE [options] inputfile [outputfile]      \n\
                                                                     \n\
  Options:                                                           \n\
     -a       triple-A quality resampling filter                     \n\
  OR:                                                                \n\
     -f NUM   rolloff Frequency (0 < freq <= 1)       [default: 0.9] \n\
     -b NUM   Beta ( >= 1)                            [default: 9.0] \n\
     -l NUM   filter Length (odd number <= 65)        [default: 65]  \n\
                                                                     \n\
     -n       No interpolation of filter coefficients (faster)       \n\
     -i       resample by linear Interpolation, not with filter      \n\
     -t       Terse (don't print out so much)                        \n\
     -v       print Version of program and quit                      \n\
  If no output file specified, writes to \"inputfile.resamp\".       \n\
"


static void
fail(char *s)
{
   fprintf(stderr, "\n*** resample: %s \n", s);  /* Display error message  */
   fprintf(stderr, USAGE);
   exit(1);                     /* Exit, indicating error */
}


static void
fails(char *s, char *s2)
{
   printf("resample: ");        /* Display error message  */
   printf(s, s2);
   printf("\n");
   exit(1);                     /* Exit, indicating error */
}


int
main(int argc, char *argv[])
{
   double factor;                 /* factor = Sndout/Sndin */
   double newsrate = 0.0;
   FiltSpec filtspec;
   BOOL   interpFilt = TRUE;      /* TRUE means interpolate filter coeffs */
   BOOL   largeFilter = FALSE;    /* TRUE means use 65-tap FIR filter */
   BOOL   linearInterp = FALSE;   /* TRUE => no filter, linearly interpolate */
   int    trace = TRUE, designFilter = FALSE;
   int    infd, outfd, insrate, nChans, inFormat, result;
   int    inCount, outCount, outCountReal;
   char   *insfname, *outsfname;
   struct stat statbuf;

   if (argc == 1) {
      fprintf(stderr, USAGE);
      exit(1);
   }

   filtspec.rolloff = filtspec.beta = 0.0;
   filtspec.length = 65;

   while (--argc && **(++argv) == '-') {
      ++(argv[0]);              /* skip over '-' */
      switch (*argv[0]) {
         case 'r':                        /* -r srate (was -to srate) */
            if (--argc)
               sscanf(*++argv, "%lf", &newsrate);
            if (newsrate <= 0.0)
               fail("srate must be greater than 0.");
            if (trace)
               printf("Target sampling-rate set to %f.\n", newsrate);
            break;
         case 'a':                        /* -a triple-A quality filter */
            largeFilter = TRUE;
            if (trace)
               printf("Choosing higher quality filter.\n");
            break;
         case 'f':                        /* -f rolloff frequency */
            if (--argc)
               sscanf(*++argv, "%lf", &filtspec.rolloff);
            if (filtspec.rolloff <= 0.0 || filtspec.rolloff > 1.0)
               fail("rolloff must be between 0 and 1.");
            designFilter = TRUE;
            break;
         case 'b':                        /* -b beta */
            if (--argc)
               sscanf(*++argv, "%lf", &filtspec.beta);
            if (newsrate <= 0.0)
               fail("srate must be greater than 0.");
            designFilter = TRUE;
            break;
         case 'l':                        /* -l filter length */
            if (--argc)
               sscanf(*++argv, "%d", &filtspec.length);
            if ((filtspec.length & 1) == 0 || filtspec.length < 1
                                           || filtspec.length > 65)
               fail("filter length must be odd and <= 65.");
            designFilter = TRUE;
            break;
         case 'n':                        /* -n no coeff interpolation */
            interpFilt = FALSE;
            if (trace)
               printf("Filter-table interpolation disabled.\n");
            break;
         case 'i':                        /* -i linear inpterp. (was -l) */
            linearInterp = TRUE;
            if (trace)
               printf("Using linear instead of bandlimited interpolation\n");
            break;
         case 't':                        /* -terse */
            trace = 0;
            break;
         case 'v':                        /* -version */
            printf(RESAMPLE_VERSION);
            exit(0);
            break;
         default:
            fprintf(stderr, "Unknown switch -%s\n", argv[0]);
            fprintf(stderr, USAGE);
            exit(1);
      }
   }

   if (newsrate == 0.0)
      fail("Must specify sampling-rate for output file via '-r' option");

   if (argc < 1)
      fail("Need to specify input sound file");
   insfname = *argv;

   if (argc < 2) {
      /* construct output file name */
      static char name[FILENAME_MAX];

      int len = FILENAME_MAX - (strlen(OUT_SUFFIX) + 1);
      strncpy(name, insfname, len);
      name[len - 1] = '\0';
      strcat(name, OUT_SUFFIX);
      outsfname = name;
   }
   else
      outsfname = *++argv;

   /* Test whether output file name exists. If so, bail... */
   result = stat(outsfname, &statbuf);
   if (result != -1)
      fails("\"%s\" already exists", outsfname);
   if (errno != ENOENT)
      fails("Error creating output file (%s)", strerror(errno));

   if (trace)
      printf("Writing output to \"%s\".\n", outsfname);

   /* Open input file and gather info from its header */
   infd = sndlib_open_read(insfname);
   if (infd == -1)
      fails("Could not open input file \"%s\"", insfname);
   if (NOT_A_SOUND_FILE(mus_header_type()))
      fails("\"%s\" is probably not a sound file.", insfname);
   inFormat = mus_header_format();
   nChans = mus_header_chans();
   insrate = mus_header_srate();
   inCount = mus_header_samples();
   inCount /= nChans;           /* to get sample frames */

   /* Compute sampling conversion factor */
   factor = newsrate / (double)insrate;
   if (trace)
      printf("Sampling rate conversion factor set to %f\n", factor);

   /* Create and open output file */
   outfd = sndlib_create(outsfname, OUT_TYPE, OUT_FORMAT, newsrate, nChans);
   if (outfd == -1)
      fails("Could not create output file \"%s\"", outsfname);
   outCount = (int)(factor * (double)inCount + .5);        /* output frames */

   if ((linearInterp && largeFilter) || (linearInterp && designFilter)) {
      designFilter = largeFilter = FALSE;
   }
   if (designFilter) {
      if (filtspec.rolloff == 0.0)
         filtspec.rolloff = DEFAULT_ROLLOFF;
      if (filtspec.beta == 0.0)
         filtspec.beta = DEFAULT_BETA;
      if (filtspec.length == 0)
         filtspec.length = DEFAULT_LENGTH;
      if (largeFilter) {
         largeFilter = FALSE;
         if (trace)
            printf("Ignoring triple-A quality filter request, since you also "
                   "want to design your own filter.\n");
      }
      if (trace) {
         printf("Attempting to design filter with these specs:\n");
         printf("   rolloff = %g, beta = %g, length = %d\n",
                filtspec.rolloff, filtspec.beta, filtspec.length);
      }
   }

   if (trace)
      printf("Starting Conversion\n");

   outCountReal = resample(factor, infd, outfd, inFormat, OUT_FORMAT,
                           inCount, outCount, nChans,
                           interpFilt, linearInterp, largeFilter,
                           designFilter ? &filtspec : NULL);
   if (outCountReal <= 0)
      fail("Conversion factor out of range");

   if (trace && (outCount != outCountReal))
      fprintf(stderr, "outCount = %d, outCountReal = %d\n",
                                                      outCount, outCountReal);

   sndlib_close(infd, FALSE, 0, 0, 0);             /* just close it */

   /* Output samps already written; just update header and close file. */
   if (sndlib_close(outfd, TRUE, OUT_TYPE, OUT_FORMAT, outCountReal * nChans))
      fails("Error closing output file (%s)", strerror(errno));

   if (trace)
      printf("\nConversion Finished:  %d output samples.\n", outCount);
   else
      printf("\n");

   exit(0);
}

