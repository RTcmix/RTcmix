#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <ugens.h>
#include <sfheader.h>
#include "denoise.h"

#define CMIX                    /* changes to original Carl src for cmix */

extern SFHEADER sfdesc[NFILES];
extern struct stat sfst[NFILES];
extern int headersize[NFILES];

#define INPUT  0                /* cmix unit numbers */
#define OUTPUT 1
#define NOISE  2

static void hamming(float *, int, int);
static void malerr(char *, int);


/*------------------------------------------------------------------------

   Denoise, by Mark Dolson, adapted for cmix by John Gibson.

   This cmix instrument was ported from the carl package, where it was
   a command-line filter program. The man page for denoise, as well as
   Dolson's comments below, is still essential reading, even though the
   interface to the cmix instrument is different. The variable names for
   the cmix pfields are the same as the names in the denoise documentation.

   Generally, you want to leave M, L and D alone. The duration of the
   excerpt from the noise reference file must be at least .25 seconds.

   First, open the input file as unit 0, the output file as unit 1,
   and the noise reference file as unit 2. (The noise reference file
   can be the same as the input file.) Denoise can only process one
   channel at a time.

   p0  input start time
   p1  input duration (use dur(0) to get total file duration)
   p2  N: number of bandpass filters (size of FFT) [must be power of 2]
   p3  M: analysis window length [N]
   p4  L: synthesis window length [M]
   p5  D: decimation factor [M/8]
   p6  b: begin time in noise reference soundfile
   p7  e: end time in noise reference soundfile
   p8  t: threshold above noise reference in dB [try 30]
   p9  s: sharpness of noise-gate turnoff [1-5]
   p10 n: number of fft frames to average over [5]
   p11 m: minimum gain of noise-gate when off, in dB [-40]
   p12 channel number (0=left, 1=right)

   NOTE: the previous version of denoise did not have the first two pfields
   listed above.  Scores for that version still work correctly with this one.

   Dolson's comments from the original source...

   Experimental noise reduction scheme using frequency-domain noise-gating.
   This program should work best in the case of high signal-to-noise with
   hiss-type noise. The algorithm is that suggested by Moorer & Berger in
   "Linear-Phase Bandsplitting: Theory and Applications" presented at the
   76th Convention 1984 October 8-11 New York of the Audio Engineering
   Society (preprint #2132) except that it uses the Weighted Overlap-Add
   formulation for short-time Fourier analysis-synthesis in place of the
   recursive formulation suggested by Moorer & Berger. The gain in each
   frequency bin is computed independently according to 

      gain = g0 + (1-g0) * [avg / (avg + th*th*nref)] ^ sh

   where avg and nref are the mean squared signal and noise respectively for
   the bin in question. (This is slightly different than in Moorer & Berger.)
   The critical parameters th and g0 are specified in dB and internally
   converted to decimal values. The nref values are computed at the start of
   the program on the basis of a noise_soundfile (specified in the command
   line) which contains noise without signal. The avg values are computed
   over a rectangular window of m FFT frames looking both ahead and behind
   the current time. This corresponds to a temporal extent of m*D/R (which
   is typically (m*N/8)/R). The default settings of N, M, and D should be
   appropriate for most uses. A higher sample rate than 16KHz might indicate
   a higher N.

------------------------------------------------------------------------*/


double
denoise(float p[], int n_args)
{
   float *input,               /* input buffer */
   *output,                    /* output buffer */
   *anal,                      /* analysis buffer */
   *nextIn,                    /* pointer to next empty word in input */
   *nextOut,                   /* pointer to next empty word in output */
   *analWindowStart,           /* pointer to start of analysis window */
   *analWindow,                /* pointer to center of analysis window */
   *synWindowStart,            /* pointer to start of synthesis window */
   *synWindow,                 /* pointer to center of synthesis window */
   *mbuf,                      /* m most recent frames of FFT */
   *nref,                      /* noise reference buffer */
   *rsum;                      /* running sum of magnitude-squared spectrum */

   int N = 1024,               /* number of points in FFT */
    Np2,                       /* N+2 */
    M = 0,                     /* length of analWindow impulse response (M=N) */
    L = 0,                     /* length of synWindow impulse response (L=M) */
    D = 0,                     /* decimation factor (default is M/8) */
    I = 0,                     /* interpolation factor (should be I=D) */
    m = 5,                     /* number of frames to save in mbuf */
    mi = 0,                    /* frame offset index in mbuf */
    mj,                        /* delayed offset index in mbuf */
    md,                        /* number of frames of delay in mbuf (m/2) */
    mp,                        /* mi * Np2 */
    analWinLen,                /* half-length of analysis window */
    synWinLen;                 /* half-length of synthesis window */

   long outCount,              /* number of samples written to output */
    ibuflen,                   /* length of input buffer */
    obuflen,                   /* length of output buffer */
    beg = 0,                   /* first sample in noise soundfile */
    end = 0,                   /* last sample in noise soundfile */
    nI,                        /* current input (analysis) sample */
    nO,                        /* current output (synthesis) sample */
    nId,                       /* delayed input (analysis) sample */
    nOd,                       /* delayed output (synthesis) sample */
//  nMin = 0,                  /* first input (analysis) sample */
    nMax = 200000000;          /* last input sample (unless EOF) */

   float Pi,                   /* 3.14159... */
    TwoPi,                     /* 2*Pi */
    gain,                      /* gain of noise gate */
    g0 = -40.,                 /* minimum gain for noise gate */
    g0m,                       /* 1. - g0 */
    th = 30.,                  /* threshold above noise reference (dB) */
    avg,                       /* average square energy */
    fac,                       /* factor in gain computation */
    minv,                      /* 1 / m */
    sum = 0.,                  /* sum for averaging */
//  rIn,                       /* decimated sampling rate */
//  rOut,                      /* pre-interpolated sampling rate */
    Ninv,                      /* 1. / N */
    R = 0.;                    /* input sampling rate */

   int i, j, k, n,             /* index variables */
    i0, i1,                    /* indices for real and imaginary FFT values */
    Dd,                        /* number of new inputs to read (Dd <= D) */
    Ii,                        /* number of new outputs to write (Ii <= I) */
    N2,                        /* N/2 */
    Mf = 0,                    /* flag for even M */
    Lf = 0,                    /* flag for even L */
    sh = 1,                    /* sharpness control for noise gate gain */
//  v = 0,                     /* flag for verifying noise reference values */
    flag;                      /* end-of-input flag */

   float srate;                /* sample rate from header on input file */
   float inskip=0.0, indur=0.0;
   long nsampsIn, nsampsNoise, samps_read;
   int inchan, outchan, nchans, noiseInChan, class, process_whole_file=0;
   float inputdur, noiseInskip, noiseInend, noiseDur;
   float noiseIn[SF_MAXCHAN], in[SF_MAXCHAN], out[SF_MAXCHAN];

   /* If 13 args, then first two are inskip and indur, followed by the
      original 11 args. */
   if (n_args == 11)
      process_whole_file = 1;
   else if (n_args == 13)
      process_whole_file = 0;
   else
      die("denoise", "Wrong number of arguments.");

   if (process_whole_file) {
      N = p[0];
      M = p[1];
      L = p[2];
      D = p[3];
      noiseInskip = p[4];
      noiseInend = p[5];
      noiseDur = p[5] - p[4];
      th = p[6];
      sh = p[7];
      m = p[8];
      g0 = p[9];
      inchan = p[10];
   }
   else {
      inskip = p[0];
      indur = p[1];
      N = p[2];
      M = p[3];
      L = p[4];
      D = p[5];
      noiseInskip = p[6];
      noiseInend = p[7];
      noiseDur = p[7] - p[6];
      th = p[8];
      sh = p[9];
      m = p[10];
      g0 = p[11];
      inchan = p[12];
   }


   /* input file */

   R = srate = sfsrate(&sfdesc[INPUT]);
   nchans = sfchans(&sfdesc[INPUT]);
   if (inchan >= nchans || inchan < 0)
      die("denoise", "You asked for channel %d of a %d-channel file.",
                                                              inchan, nchans);
   class = sfclass(&sfdesc[INPUT]);
   if (process_whole_file)
      inputdur = (float)(sfst[INPUT].st_size - headersize[INPUT])
                                      / (float)class / (float)nchans / srate;
   else
      inputdur = indur;
   nsampsIn = setnote(inskip, inputdur, INPUT);


   /* output file */

   if (sfsrate(&sfdesc[OUTPUT]) != srate)
      die("denoise", "Output and input files must have same sample rate.");

   if (sfchans(&sfdesc[OUTPUT]) == nchans)
      outchan = inchan;
   else {
      outchan = 0;
      rtcmix_warn("denoise", "Writing to channel 0 of output file, since output and "
                      "input files don't have the same number of channels.");
   }
   setnote(0., inputdur, OUTPUT);


   /* reference noise file */

   if (sfsrate(&sfdesc[NOISE]) != srate) {
      die("denoise",
             "Noise reference file must have same sample rate as input file.");
   }
   if (sfchans(&sfdesc[NOISE]) == nchans)
      noiseInChan = inchan;            /* same as input chan */
   else
      noiseInChan = 0;                 /* just take left chan */
   if (noiseInend > (float)(sfst[NOISE].st_size - headersize[NOISE])
                    / (float)sfclass(&sfdesc[NOISE]) / (float)nchans / srate) {
      die("denoise", "Noise reference file end time is past end of file.");
   }
   if (noiseInend <= noiseInskip) {
      die("denoise",
                  "Make sure noise file begin time is earlier than end time!");
   }
   if (noiseDur < .25) {
      rtcmix_warn("denoise", "Try to have at least .25 seconds of reference noise.");
      /* but keep going */
   }
   nsampsNoise = setnote(noiseInskip, noiseDur, NOISE);
   beg = (long)(noiseInskip * srate + 0.5);      /* first _frame_ to read */
   end = (long)(noiseInend * srate + 0.5);       /* last _frame_ to read */

   rtcmix_advise("denoise", "Processing channel %d...", inchan);


   /* Set up parameter values */

   for (N2 = 2; N2 <= 16384; N2 *= 2)
      if (N2 >= N)
         break;

   if (N2 > 16384)
      die("denoise", "N too large.");

   N = N2;
   N2 = N / 2;
   Ninv = 1. / N;
   Np2 = N + 2;

   if (M == 0)
      M = N;
   if ((M % 2) == 0)
      Mf = 1;
   if (L == 0)
      L = M;
   if ((L % 2) == 0)
      Lf = 1;

   ibuflen = 4 * M;
   obuflen = 4 * L;

   if (D == 0)
      D = ((float) M / 8.);
   I = D;

   Pi = 4. * atan(1.);
   TwoPi = 2. * Pi;

   minv = 1. / m;
   md = m / 2;
   g0 = pow(10., (.05 * g0));
   g0m = 1. - g0;
   th = pow(10., (.05 * th));


/* set up analysis window: The window is assumed to be symmetric
   with M total points.  After the initial memory allocation,
   analWindow always points to the midpoint of the window (or
   one half sample to the right, if M is even); analWinLen is
   half the true window length (rounded down). If the window
   duration is longer than the transform (M > N), then the window
   is multiplied by a sin(x)/x function to meet the condition:
   analWindow[Ni] = 0 for i != 0.  In either case, the window
   is renormalized so that the amplitude estimates are properly
   scaled.  The maximum allowable window duration is ibuflen/2. */

   analWindow = (float *)calloc(M + Mf, sizeof(float));
   if (analWindow == NULL)
      malerr("denoise: insufficient memory", 1);
   analWindowStart = analWindow;            /* for free() below */
   analWindow += (analWinLen = M / 2);

   hamming(analWindow, analWinLen, Mf);
   for (i = 1; i <= analWinLen; i++)
      *(analWindow - i) = *(analWindow + i - Mf);

   if (M > N) {
      if (Mf)
         *analWindow *= N * sin((double) Pi * .5 / N) / (Pi * .5);
      for (i = 1; i <= analWinLen; i++)
         *(analWindow + i) *=
             N * sin((double) Pi * (i + .5 * Mf) / N) / (Pi * (i + .5 * Mf));
      for (i = 1; i <= analWinLen; i++)
         *(analWindow - i) = *(analWindow + i - Mf);
   }

   sum = 0.;
   for (i = -analWinLen; i <= analWinLen; i++)
      sum += *(analWindow + i);

   sum = 2. / sum;            /* factor of 2 comes in later in trig identity */

   for (i = -analWinLen; i <= analWinLen; i++)
      *(analWindow + i) *= sum;


/* set up synthesis window:  For the minimal mean-square-error
   formulation (valid for N >= M), the synthesis window
   is identical to the analysis window (except for a
   scale factor), and both are even in length.  If N < M,
   then an interpolating synthesis window is used. */

   synWindow = (float *)calloc(L + Lf, sizeof(float));
   if (synWindow == NULL)
      malerr("denoise: insufficient memory", 1);
   synWindowStart = synWindow;            /* for free() below */
   synWindow += (synWinLen = L / 2);

   if (M <= N) {
      hamming(synWindow, synWinLen, Lf);
      for (i = 1; i <= synWinLen; i++)
         *(synWindow - i) = *(synWindow + i - Lf);

      for (i = -synWinLen; i <= synWinLen; i++)
         *(synWindow + i) *= sum;

      sum = 0.;
      for (i = -synWinLen; i <= synWinLen; i += I)
         sum += *(synWindow + i) * *(synWindow + i);

      sum = 1. / sum;

      for (i = -synWinLen; i <= synWinLen; i++)
         *(synWindow + i) *= sum;
   }
   else {
      hamming(synWindow, synWinLen, Lf);
      for (i = 1; i <= synWinLen; i++)
         *(synWindow - i) = *(synWindow + i - Lf);

      if (Lf)
         *synWindow *= I * sin((double) Pi * .5 / I) / (Pi * .5);
      for (i = 1; i <= synWinLen; i++)
         *(synWindow + i) *=
             I * sin((double) Pi * (i + .5 * Lf) / I) / (Pi * (i + .5 * Lf));
      for (i = 1; i <= synWinLen; i++)
         *(synWindow - i) = *(synWindow + i - Lf);

      sum = 1. / sum;

      for (i = -synWinLen; i <= synWinLen; i++)
         *(synWindow + i) *= sum;
   }


/* set up input buffer:  nextIn always points to the next empty
   word in the input buffer (i.e., the sample following
   sample number (n + analWinLen)).  If the buffer is full,
   then nextIn jumps back to the beginning, and the old
   values are written over. */

   input = (float *)calloc(ibuflen, sizeof(float));
   if (input == NULL)
      malerr("denoise: insufficient memory", 1);

   nextIn = input;


/* set up output buffer:  nextOut always points to the next word
   to be shifted out.  The shift is simulated by writing the
   value to the standard output and then setting that word
   of the buffer to zero.  When nextOut reaches the end of
   the buffer, it jumps back to the beginning.  */

   output = (float *)calloc(obuflen, sizeof(float));
   if (output == NULL)
      malerr("denoise: insufficient memory", 1);

   nextOut = output;


/* set up analysis buffer for (N/2 + 1) channels: The input is real,
   so the other channels are redundant. */

   anal = (float *)calloc(Np2, sizeof(float));
   if (anal == NULL)
      malerr("denoise: insufficient memory", 1);

/* noise reduction: calculate noise reference by taking as many 
   consecutive FFT's as possible in noise soundfile, and
   averaging them all together.  Multiply by th*th to
   establish threshold for noise-gating in each bin. */

   nref = (float *)calloc((N2 + 1), sizeof(float));
   if (nref == NULL)
      malerr("denoise: insufficient memory", 1);

   mbuf = (float *)calloc(m * Np2, sizeof(float));
   if (mbuf == NULL)
      malerr("denoise: insufficient memory", 1);

   rsum = (float *)calloc(N2 + 1, sizeof(float));
   if (rsum == NULL)
      malerr("denoise: insufficient memory", 1);


   k = 0;
   j = beg;
   while (j < end) {
      for (i = 0; i < N; i++, j++) {
#ifdef CMIX
         if (j < end) {
            GETIN(noiseIn, NOISE);
            anal[i] = noiseIn[noiseInChan];
         }
#else
         if (j < end)
            anal[i] = fsndi(sfd, j);
         if (sferror)
            quit();
#endif
      }

      cfast(anal, N);

      for (i = 0; i <= N + 1; i++)
         anal[i] *= Ninv;

      for (i = 0; i <= N2; i++) {
         fac = anal[2 * i] * anal[2 * i];
         fac += anal[2 * i + 1] * anal[2 * i + 1];
         nref[i] += fac;
      }
      k++;
   }
   fac = th * th / k;
   for (i = 0; i <= N2; i++)
      nref[i] *= fac;

/* initialization: input time starts negative so that the rightmost
   edge of the analysis filter just catches the first non-zero
   input samples; output time is always T times input time.
   nI and nO count time with regard to input, while nId and
   nOd count time according to the output.  The latter are all
   that really matter except for catching the EOF on input. */

   outCount = 0;
   nI = -(analWinLen / D) * D;  /* input time (in samples) */
   nId = nI - md * D;           /* subtract additional delay in mbuf */
   nO = nI;
   nOd = nId;
   Dd = analWinLen + nI + 1;    /* number of new inputs to read */
   Ii = 0;                      /* number of new outputs to write */
   flag = 1;
   mi = 0;
   mj = m - md;
   if (mj >= m)
      mj = 0;
   mp = mi * Np2;

/* main loop:  If nMax is not specified it is assumed to be very large
   and then readjusted when getfloat detects the end of input. */

   samps_read = 0;
   while (nId < (nMax + analWinLen)) {

      for (i = 0; i < Dd; i++) {
#ifdef CMIX
         if (!GETIN(in, INPUT))
            Dd = i;             /* EOF ? */
         if (++samps_read > nsampsIn)
            Dd = i;
         *nextIn++ = in[inchan];
#else
         if (getfloat(nextIn++) <= 0)
            Dd = i;             /* EOF ? */
#endif
         if (nextIn >= (input + ibuflen))
            nextIn -= ibuflen;
      }

      if (nI > 0)
         for (i = Dd; i < D; i++) {  /* zero fill at EOF */
            *(nextIn++) = 0.;
            if (nextIn >= (input + ibuflen))
               nextIn -= ibuflen;
         }


      /* analysis: The analysis subroutine computes the complex output at
         time n of (N/2 + 1) of the FFT bins.  It operates on input
         samples (n - analWinLen) thru (n + analWinLen) and expects
         to find these in input[(n +- analWinLen) mod ibuflen].
         It expects analWindow to point to the center of a
         symmetric window of length (2 * analWinLen +1).  It is the
         responsibility of the main program to ensure that these values
         are correct!  The results are returned in anal as succesive
         pairs of real and imaginary values for the lowest (N/2 + 1)
         channels. */


      for (i = 0; i < N + 2; i++)
         *(anal + i) = 0.;      /*initialize */

      j = (nI - analWinLen - 1 + ibuflen) % ibuflen;  /*input pntr */

      k = nI - analWinLen - 1;  /*time shift */
      while (k < 0)
         k += N;
      k = k % N;

      for (i = -analWinLen; i <= analWinLen; i++) {
         if (++j >= ibuflen)
            j -= ibuflen;
         if (++k >= N)
            k -= N;
         anal[k] += analWindow[i] * input[j];
      }

      cfast(anal, N);

      /* noise reduction: for each bin, calculate average magnitude-squared
         and calculate corresponding gain.  Apply this gain to delayed
         FFT values in mbuf[mj*Np2 + i?]. */

      for (i = 0; i <= N2; i++) {
         i0 = 2 * i;
         i1 = i0 + 1;
         rsum[i] -= mbuf[mp + i0] * mbuf[mp + i0];
         rsum[i] -= mbuf[mp + i1] * mbuf[mp + i1];
         rsum[i] += anal[i0] * anal[i0];
         rsum[i] += anal[i1] * anal[i1];
         avg = minv * rsum[i];
         if (avg < 0.)
            avg = 0.;
         if (avg == 0.)
            fac = 0.;
         else
            fac = avg / (avg + nref[i]);
         for (j = 1; j < sh; j++)
            fac *= fac;
         gain = g0m * fac + g0;
         mbuf[mp + i0] = anal[i0];
         mbuf[mp + i1] = anal[i1];
         anal[i0] = gain * mbuf[mj * Np2 + i0];
         anal[i1] = gain * mbuf[mj * Np2 + i1];
      }

      if (++mi >= m)
         mi = 0;
      if (++mj >= m)
         mj = 0;
      mp = mi * Np2;

      /* synthesis: The synthesis employs the Weighted Overlap-Add
         technique to reconstruct the time-domain signal.  The
         (N/2 + 1) filter outputs at time n are inverse Fourier
         transformed, windowed, and added into the output array.  The
         subroutine thinks of output as a shift register in which 
         locations are referenced modulo obuflen.  Therefore, the main
         program must take care to zero each location which it "shifts"
         out (to standard output). */

      cfsst(anal, N);

      j = nOd - synWinLen - 1;
      while (j < 0)
         j += obuflen;
      j = j % obuflen;

      k = nOd - synWinLen - 1;
      while (k < 0)
         k += N;
      k = k % N;

      for (i = -synWinLen; i <= synWinLen; i++) {  /*overlap-add */
         if (++j >= obuflen)
            j -= obuflen;
         if (++k >= N)
            k -= N;
         output[j] += anal[k] * synWindow[i];
      }


      for (i = 0; i < Ii; i++) {  /* shift out next Ii values */
#ifdef CMIX
         for (n = 0; n < SF_MAXCHAN; n++)
            out[n] = 0.0;
         out[outchan] = *nextOut;
         ADDOUT(out, OUTPUT);
#else
         putfloat(nextOut);
#endif
         *(nextOut++) = 0.;
         if (nextOut >= (output + obuflen))
            nextOut -= obuflen;
         outCount++;
      }

      if (flag)
         if ((nI > 0) && (Dd < D)) {  /* EOF detected */
            flag = 0;
            nMax = nI + analWinLen - (D - Dd);
         }

      nI += D;                  /* increment time */
      nO += I;
      nId += D;                 /* increment time */
      nOd += I;

      if ((nI + analWinLen) <= nMax)
         Dd = D;
      else if ((nI + analWinLen - D) <= nMax)
         Dd = nMax - (nI + analWinLen - D);
      else
         Dd = 0;

      if (nOd > (synWinLen + I))
         Ii = I;
      else if (nOd > synWinLen)
         Ii = nOd - synWinLen;
      else {
         Ii = 0;
         for (i = nOd + synWinLen; i < obuflen; i++)
            if (i > 0)
               output[i] = 0.;
      }
   }

   while (outCount <= nMax) {
      outCount++;
#ifdef CMIX
      for (n = 0; n < SF_MAXCHAN; n++)
         out[n] = 0.0;
      out[outchan] = *nextOut++;
      ADDOUT(out, OUTPUT);
#else
      putfloat(nextOut++);
#endif
      if (nextOut >= (output + obuflen))
         nextOut -= obuflen;
   }

#ifdef CMIX
   endnote(OUTPUT);

   free(analWindowStart);
   free(synWindowStart);
   free(input);
   free(output);
   free(anal);
   free(nref);
   free(mbuf);
   free(rsum);
#else
   flushfloat();
#endif

   return 0.0;
}


#ifndef CMIX
usage(exitcode)
int exitcode;
{
   fprintf(stderr, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
      "usage: denoise [flags] noise_soundfile < floatsams > floatsams\n",
           "input and output must be files or pipes\n",
           "flags:\n",
           "R = input sample rate (automatically read from stdin)\n",
           "N = # of bandpass filters (size of FFT) (1024)\n",
           "M = analysis window length (N-1)\n",
           "L = synthesis window length (M) \n",
           "D = decimation factor (M/8)\n",
           "b = begin time in noise reference soundfile (0)\n",
           "e = end time in noise reference soundfile (end)\n",
           "d = duration in noise reference soundfile (end - begin)\n",
           "t = threshold above noise reference in dB (30)\n",
           "s = sharpness of noise-gate turnoff (1) (1 to 5)\n",
           "n = number of FFT frames to average over (5)\n",
           "m = minimum gain of noise-gate when off in dB (-40)\n",
           "filename must follow all flags\n"
       );

   exit(exitcode);
}
#endif


static void
hamming(float *win, int winLen, int even)
{
   float Pi, ftmp;
   int i;

   Pi = 4. * atan(1.);
   ftmp = Pi / winLen;

   if (even) {
      for (i = 0; i < winLen; i++)
         *(win + i) = .54 + .46 * cos(ftmp * (i + .5));
      *(win + winLen) = 0.;
   }

   else {
      *(win) = 1.;
      for (i = 1; i <= winLen; i++)
         *(win + i) = *(win - i) = .54 + .46 * cos(ftmp * i);
   }
}


static void
malerr(char *str, int ex)
{
   fprintf(stderr, "%s\n", str);
   exit(ex);
}


#ifndef CMIX
quit()
{
   fprintf(stderr, "exiting\n");
// (void) sfallclose(); 
   exit(1);
}
#endif


int
profile()
{
   UG_INTRO("denoise", denoise);
   return 0;
}


