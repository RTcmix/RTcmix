/* now problem is that for >1 chan scan, hist will show || of all channels */
/* ok-->that's not a bug, it's a feature! */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <errno.h>
#include "../genlib/complexf.h"
#include <byte_routines.h>
#include <sfheader.h>
#include <sndlibsupport.h>

#define  BUFSIZE 32768
/* use smaller buffers for hist across network */
#define  ABS(x) ((x < 0) ? (-x) : (x))

SFHEADER sfh;
int bytestoread;

extern int swap;                /* defined in sys/check_byte_order.c */

extern int fft(long, long, complex[]);

/* local function prototypes */
static float rmsshort(short *, int, float *, int, int);
static float rmsfloat(float *, int, float *, int, int);
static float scanshort(short *, int, float *, int, int);
static float scanfloat(float *, int, float *, int, int);
static void fftprint(float *, int);
static void sortout(char *, int, float);
static void shutup(void);

#define DEFAULT_STARS  40

#define MIN_FFT_SIZE   32
#define MAX_FFT_SIZE   2048

#define INTRO_MESSAGE "hist: type 'h' for help, 'q' to quit\n"

#define PROMPT "\nEnter <increment> <start_time> <end_time> [q to quit]: "



void
runtime_usage()
{
   printf("\n\
        time_increment     start_time      end_time       (seconds)       \n\
   OR:  -nsamps_increment  -start_samp     -nsamps_dur    (frame numbers) \n\
                                                                          \n\
   - For RMS amplitudes, make either start_time OR end_time negative.     \n\
   - Shows all chans by default. To show a range of chans, append this:   \n\
        first_channel      last_channel    (starting from 0)              \n\
   - Takes overall peak from header. To override, give after chan range.  \n\
   - To plot an FFT...                                                    \n\
        size_of_fft        start_time      channel                        \n\
             ^------ (power of 2 btw. %d and %d)   \
\n", MIN_FFT_SIZE, MAX_FFT_SIZE);
}


void
cmdline_usage()
{
   printf("\
Usage:  hist [options] filename                                     \n\
        options:  -p   (no star plots)                              \n\
                  -n   (max number of stars per line - default: 40) \n\
Run-time commands: ");     /* no newline */
   runtime_usage();
   exit(0);
}


int main(int argc, char *argv[])
{
   int i, j, jj, bytes, sf, result, loopsize, noplot, fftflag;
   int bytenumber, next, total, segments, loopbytes, rmsflag;
   int headersize, size, auto_advance, chfirst, chlast;
   int nskips, nprints, nstars, sampleflag;
   SFMAXAMP sfm;
   struct stat sfst;
   char *cp, *sfname, *str, *getsfcode();
   short *ipoint;
   float *xpoint;
   float dur, sample, jpeak, opeak;
   double incr, start, end, tmpstart;
   char buffer[BUFSIZE];
   float output[65536];
   complex s[8192];
   float maxamps[SF_MAXCHAN];

   segments=0;
   rmsflag=0;
   sampleflag=0;
   sample=0.0;

   if (argc == 1) {
      cmdline_usage();
      exit(0);
   }

   printf("%s\n", INTRO_MESSAGE);

   noplot = 0;
   nstars = DEFAULT_STARS;

   while ((*++argv)[0] == '-') {
      argc -= 2;                /* Take away two args */
      for (cp = argv[0] + 1; *cp; cp++) {
         switch (*cp) {         /* Grap options */
            case 'p':
               noplot = 1;
               break;
            case 'n':
               nstars = atoi(*++argv);
               break;
            default:
               cmdline_usage();
         }
      }
   }
   if (nstars != DEFAULT_STARS)
      printf("using %d stars per line, max\n", nstars);

   sfname = argv[0];
   rwopensf(sfname, sf, sfh, sfst, "hist", result, 0);
   if (result < 0) {
      close(sf);
      exit(1);
   }
   headersize = getheadersize(&sfh);
   cp = getsfcode(&sfh, SF_MAXAMP);
   if (cp != NULL)
      bcopy(cp + sizeof(SFCODE), (char *)&sfm, sizeof(SFMAXAMP));

   dur = (float) (sfst.st_size - headersize)
             / (float) sfclass(&sfh) / (float) sfchans(&sfh) / sfsrate(&sfh);

   printf("%s\n", sfname);
   printsf(&sfh);
   printf("duration: %f\n", dur);
   fflush(stdout);

   opeak = 0;

   fftflag = auto_advance = 0;

   while (1) {
      printf("%s", PROMPT);

      str = fgets(buffer, 256, stdin);

      if (str == NULL || buffer[0] == 'q') {     /* ^D or 'q' quits */
         printf("\n");
         exit(0);
      }

      if (buffer[0] == 'h') {                    /* help me */
         runtime_usage();
         continue;
      }

      if (buffer[0] == '\n' && auto_advance) {
         if (fftflag)
            start += incr / sfsrate(&sfh);
         else {
            double bufdur = end - start;
            start += bufdur;
            end += bufdur;
         }
      }
      else {
         buffer[strlen(buffer)] = '\0';          /* chop newline */

         i = sscanf(buffer, "%lf %lf %lf %d %d %f",
                    &incr, &start, &end, &chfirst, &chlast, &opeak);

         if (i < 3) {
            auto_advance = 0;
            continue;                            /* not enough args */
         }
         auto_advance = 1;                       /* hit return to advance */

         if (i == 3) {
            chfirst = 0;
            chlast = sfchans(&sfh) - 1;
         }
         if (i == 4)
            chlast = chfirst;
         if (i < 6) {
            for (i = chfirst; i <= chlast; i++)
               if (sfmaxamp(&sfm, i) > opeak)
                  opeak = sfmaxamp(&sfm, i);
         }
         if (!opeak)
            opeak = 32678;         /* default for files with no amp */

         /* if increments is < 0 it means specify incr in samplesize
            if starting time is < 0 it means sample number
            if duration < 0 it indicates number of samples to look at
            if incr >= 32 we are doing fft, end arg is now channel #
         */
         rmsflag = 0;
         if (incr > 0 && (start < 0 || end < 0)) {
            rmsflag = 1;
            start = ABS(start);
            end = ABS(end);
            printf(" Plotting maximum RMS amplitudes:\n");
         }
         sampleflag = (incr < 0) ? 1 : 0;
         if (incr < 0.)
            incr = -incr / sfsrate(&sfh);
         if (start < 0.)
            start = -start / sfsrate(&sfh);
         if (end < 0.)
            end = start - (end / sfsrate(&sfh));
      }

      if (incr >= MIN_FFT_SIZE) {
         if (incr > MAX_FFT_SIZE) {
            printf("FFT size too large\n");
            continue;
         }
         fftflag = 1;
         printf(" Plotting FFT - start time: %f\n", start);
      }
      else
         fftflag = 0;

      if (!fftflag && end <= start) {
         printf("<start time> must be less than <end time>\n");
         continue;
      }

      bytes = start * sfclass(&sfh) * sfchans(&sfh) * sfsrate(&sfh);
      bytes -= bytes % (sfclass(&sfh) * sfchans(&sfh));

      /* roundout to multiple of sampleblocksize */
      loopsize = fftflag ? incr : incr * sfsrate(&sfh) + .5;
      loopbytes = loopsize * sfclass(&sfh) * sfchans(&sfh);

      /* sflseek (sfheader.h) assumes header size, so can't use it */
      bytenumber = lseek(sf, bytes + sfdatalocation(&sfh), SEEK_SET);
      if (bytenumber < 0) {
         printf("bad lseek on file %s\n", sfname);
         exit(1);
      }

      bytenumber -= headersize;
      bytestoread = fftflag ? incr * sfclass(&sfh) * sfchans(&sfh)
               : sfsrate(&sfh) * sfclass(&sfh) * sfchans(&sfh) * (end - start);

      tmpstart = start;

      /* lets user stop a huge printout with ^C */
      signal(SIGINT, (void *)shutup);

      while (bytestoread > 0) {
         total = next = (bytestoread > loopbytes) ? loopbytes : bytestoread;
         for (i = 0; i < sfchans(&sfh); i++)
            maxamps[i] = 0;
         while (next > 0) {
            segments = (next > BUFSIZE) ? BUFSIZE : next;
            if (read(sf, buffer, segments) != segments) {
               fprintf(stderr, "Reached EOF!\n");
               auto_advance = 0;
               goto nextstep;
            }
            bytenumber += segments;
            if (sfclass(&sfh) == SF_SHORT) {
               if (rmsflag)
                  sample = rmsshort((short *)buffer, segments, maxamps,
                                                             chfirst, chlast);
               else
                  sample = scanshort((short *)buffer, segments, maxamps,
                                                             chfirst, chlast);
            }
            else {
               if (rmsflag)
                  sample = rmsfloat((float *)buffer, segments, maxamps,
                                                             chfirst, chlast);
               else
                  sample = scanfloat((float *)buffer, segments, maxamps,
                                                             chfirst, chlast);
            }
            next -= segments;
         }
         bytestoread -= total;
         if (sampleflag) {
            /* print with + & - along central axis */
            printf("%6d %+e ", (bytenumber - segments)
                                     / sfclass(&sfh) / sfchans(&sfh), sample);
            if (!noplot) {
               nprints = (float)nstars / 2 * ABS(sample) / opeak;
               nskips = (sample < 0.) ? (nstars / 2) - nprints : nstars / 2;
               printf("\t");
               for (j = 0; j < nskips; j++)
                  printf(" ");
               for (j = 0; j < nprints; j++)
                  printf("*");
            }
            printf("\n");
            fflush(stdout);
         }
         else if (fftflag) {
            size = (int)incr;
            sortout(buffer, size, end);
            if (sfclass(&sfh) == SF_SHORT) {
               ipoint = (short *)buffer;
               for (jj = 0; jj < size; jj++)
                  s[jj].re = ipoint[jj];
            }
            else {
               xpoint = (float *)buffer;
               for (jj = 0; jj < size; jj++)
                  s[jj].re = xpoint[jj];
               s[jj].im = 0;
            }
            fft(1, size, s);
            for (jj = 0; jj < size; jj++)
               output[jj] = s[jj].re;
            fftprint(output, size);
            goto nextstep;
         }
         else {
            jpeak = 0;
            printf("%9.4f ", tmpstart);
            tmpstart += incr;
            for (i = chfirst; i <= chlast; i++) {
               printf("%+e ", maxamps[i]);
               jpeak = (maxamps[i] > jpeak) ?
                   maxamps[i] : jpeak;
            }
            if (!noplot) {
               nprints = (float)nstars * jpeak / opeak;
               if (rmsflag)
                  nprints *= 2;
               if (nprints <= nstars)
                  for (j = 0; j < nprints; j++)
                     printf("*");
               else
                  for (j = 0; j < nstars; j++)
                     printf(">");
            }
            printf("\n");
            fflush(stdout);
         }
      }
    nextstep:
      fflush(stdout);           /*  RK was here */
      signal(SIGINT, SIG_DFL);
   }

   return 0;
}


static float
rmsshort(short *buffer, int segments, float *maxamps, int chfirst, int chlast)
{
   int i, j, sampleno, samplesize;

   samplesize = segments / (SF_SHORT * sfchans(&sfh));
   *maxamps = *(maxamps + 1) = 0;
   for (i = 0, sampleno = 0; i < samplesize; i += sfchans(&sfh)) {
      for (j = chfirst; j <= chlast; j++) {
         if (swap) {
            byte_reverse2(&buffer[sampleno + j]);
         }
         maxamps[j] += buffer[sampleno + j] * buffer[sampleno + j];
      }
      sampleno += sfchans(&sfh);
   }
   for (j = chfirst; j <= chlast; j++)
      maxamps[j] = sqrt(maxamps[j] / (float)samplesize);
   return (float)buffer[chfirst];     /* return first value for sample scan */
}


static float
rmsfloat(float *buffer, int segments, float *maxamps, int chfirst, int chlast)
{
   int i, j, sampleno, samplesize;

   samplesize = segments / (SF_FLOAT * sfchans(&sfh));
   *maxamps = *(maxamps + 1) = 0;
   for (i = 0, sampleno = 0; i < samplesize; i += sfchans(&sfh)) {
      for (j = chfirst; j <= chlast; j++) {
         if (swap) {
            byte_reverse4(&buffer[sampleno + j]);
         }
         maxamps[j] += buffer[sampleno + j] * buffer[sampleno + j];
      }
      sampleno += sfchans(&sfh);
   }
   for (j = chfirst; j <= chlast; j++)
      maxamps[j] = sqrt(maxamps[j] / (float)samplesize);
   return buffer[chfirst];
}


static float
scanshort(short *buffer, int segments, float *maxamps, int chfirst, int chlast)
{
   int i, j, sampleno, samplesize;

   samplesize = segments / SF_SHORT;
   for (i = 0, sampleno = 0; i < samplesize; i += sfchans(&sfh)) {
      for (j = chfirst; j <= chlast; j++) {
         if (swap) {
            byte_reverse2(&buffer[sampleno + j]);
         }
         if (ABS(buffer[sampleno + j]) > maxamps[j]) {
            maxamps[j] = ABS(buffer[sampleno + j]);
         }
      }
      sampleno += sfchans(&sfh);
   }
   return (float)buffer[chfirst];     /* return first value for sample scan */
}


static float
scanfloat(float *buffer, int segments, float *maxamps, int chfirst, int chlast)
{
   int i, j, sampleno, samplesize;

   samplesize = segments / SF_FLOAT;
   for (i = 0, sampleno = 0; i < samplesize; i += sfchans(&sfh)) {
      for (j = chfirst; j <= chlast; j++) {
         if (swap) {
            byte_reverse4(&buffer[sampleno + j]);
         }
         if (ABS(buffer[sampleno + j]) > maxamps[j]) {
            maxamps[j] = ABS(buffer[sampleno + j]);
         }
      }
      sampleno += sfchans(&sfh);
   }
   return buffer[chfirst];
}


#ifdef NOT_USED
scanfloat2(buf, segments, maxamps, chfirst, chlast)
float *maxamps;
char *buf;
{
   register float *fbuffer = (float *) buf;
   long samples;
   float val;
   register int i;
   int chans;

   samples = segments / SF_FLOAT;
   chans = sfchans(&sfh);

   while (samples > 0) {
      for (i = chfirst; i <= chlast; i++) {
         val = ABS(*fbuffer);
         if (val > maxamps[i]) {
            maxamps[i] = val;
         }
         fbuffer++;
      }
      samples -= chans;
   }
}
#endif


static void shutup()
{
   bytestoread = 0;
}


/* sort out relevant channel and remove dcbias */
static void
sortout(char *buffer, int size, float end)
{
   float *fpoint, aver;
   short *ipoint;
   int i, j;        /* end is actually the number of the channel to look at */
   int channel = (int)end + .01;

   if (sfclass(&sfh) == 2) {
      ipoint = (short *)buffer;
      for (i = 0, j = channel, aver = 0; i < size; i++, j += sfchans(&sfh))
         aver += ipoint[i] = ipoint[j];
      aver /= (float)size;
      for (i = 0; i < size; i++)
         ipoint[i] -= aver;
   }
   else {
      fpoint = (float *)buffer;
      for (i = 0, j = channel, aver = 0; i < size; i++, j += sfchans(&sfh))
         aver += fpoint[i] = fpoint[j];
      aver /= (float)size;
      for (i = 0; i < size; i++)
         fpoint[i] -= aver;
   }
}


static void
fftprint(float *output, int size)
{
   int i, j;
   float peak = 0;
   float nstars = 40.;
   bytestoread = 1;

   for (i = 0; i < size / 2; i++)
      if (output[i] > peak)
         peak = output[i];
   for (i = 0; i < size / 2; i++) {
      printf("%6.0f %6.0f ", sfsrate(&sfh) * (float)i / (float)size, output[i]);
      for (j = 0; j < (nstars * output[i] / peak); j++)
         printf("*");
      printf("\n");
      if (!bytestoread)
         return;
   }
}

