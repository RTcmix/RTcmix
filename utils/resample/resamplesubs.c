/* resamplesubs.c - sampling rate conversion subroutines */
// Altered version

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include "resample.h"
#include "smallfilter.h"
#include "largefilter.h"
#include "filterkit.h"
#include <sndlibsupport.h>
#include <byte_routines.h>

#define ATTENUATE_SCALE_FACTOR

#define IBUFFSIZE 4096    /* Input buffer size */


/* CAUTION: Assumes we call this only for one resample job. */
/* return: 0 - notDone */
/*        >0 - index of last sample */
static int
readData(int   infd,          /* input file descriptor */
         int   inDataFormat,  /* sndlib data format code */
         int   inCount,       /* _total_ number of frames in input file */
         HWORD *outPtr1,      /* array receiving left chan samps */
         HWORD *outPtr2,      /* array receiving right chan samps */
         int   dataArraySize, /* size of these arrays (in frames) */
         int   nChans,
         int   Xoff)          /* read into input array starting at this index */
{
   int      i, n, nFrames, swap, bytesPerSamp;
   ssize_t  bytesToRead, bytesRead;
   char     *bufp;
   static unsigned int framecount;  /* frames previously read */
   static short *ibuf = NULL;

   bytesPerSamp = sizeof(short);

   if (ibuf == NULL) {             /* first time called, so allocate it */
      ibuf = (short *) calloc(dataArraySize * nChans, bytesPerSamp);
      if (ibuf == NULL) {
         fprintf(stderr, "readData: Can't allocate input buffer!\n");
         exit(1);
      }
      framecount = 0;               /* init this too */
   }

   nFrames = dataArraySize - Xoff;   /* Calculate number of samples to get */
   outPtr1 += Xoff;                 /* Start at designated sample number */
   outPtr2 += Xoff;

   bufp = (char *) ibuf;

   bytesToRead = nFrames * nChans * bytesPerSamp;

   while (bytesToRead > 0) {
      bytesRead = read(infd, bufp, bytesToRead);
      if (bytesRead == -1) {
         perror("readData (read)");
         exit(1);
      }
      if (bytesRead == 0)          /* EOF */
         break;

      bufp += bytesRead;
      bytesToRead -= bytesRead;
   }

   /* If we reached EOF, zero out remaining part of buffer that we
      expected to fill.
   */
   while (bytesToRead > 0) {
      (* (short *) bufp) = 0;
      bufp += bytesPerSamp;
      bytesToRead -= bytesPerSamp;
   }

#if MUS_LITTLE_ENDIAN
   swap = IS_BIG_ENDIAN_FORMAT(inDataFormat);
#else
   swap = IS_LITTLE_ENDIAN_FORMAT(inDataFormat);
#endif

   if (nChans == 1) {
      if (swap) {
         for (i = 0; i < nFrames; i++) {
            ibuf[i] = reverse_int2(&ibuf[i]);
            *outPtr1++ = (HWORD) ibuf[i];
         }
      }
      else {
         for (i = 0; i < nFrames; i++)
            *outPtr1++ = (HWORD) ibuf[i];
      }
   }
   else {  /* nChans = 2 */
      int nSamps = nFrames * nChans;
      if (swap) {
         for (i = 0; i < nSamps; i += nChans) {
            ibuf[i] = reverse_int2(&ibuf[i]);
            *outPtr1++ = (HWORD) ibuf[i];
            ibuf[i + 1] = reverse_int2(&ibuf[i + 1]);
            *outPtr2++ = (HWORD) ibuf[i + 1];
         }
      }
      else {
         for (i = 0; i < nSamps; i += nChans) {
            *outPtr1++ = (HWORD) ibuf[i];
            *outPtr2++ = (HWORD) ibuf[i + 1];
         }
      }
   }

   framecount += nFrames;

   if (framecount >= inCount)            /* return index of last samp */
      return (((nFrames - (framecount - inCount)) - 1) + Xoff);
   else
      return 0;
}


static int
writeData(int   outfd,          /* output file descriptor */
          int   outDataFormat,  /* sndlib data format code for output file */
          HWORD *outPtr1,       /* array containing left chan samps */
          HWORD *outPtr2,       /* array containing right chan samps */
          int   dataArraySize,  /* size of these arrays (in frames) */
          int   nChans)
{
   int i, bytesPerSamp, nBytes;
   unsigned char *bufp;
   static short *obuf = NULL;

   bytesPerSamp = sizeof(short);

   if (obuf == NULL) {             /* first time called, so allocate it */
      obuf = (short *) calloc(dataArraySize * nChans, bytesPerSamp);
      if (obuf == NULL) {
         fprintf(stderr, "writeData: Can't allocate output buffer!\n");
         exit(1);
      }
   }

   /* Copy & byteswap samples from output arrays to interleaved file buffer. */

   bufp = (unsigned char *) obuf;

   switch (outDataFormat) {
      case MUS_BSHORT:
         if (nChans == 1) {
            for (i = 0; i < dataArraySize; i++) {
               m_set_big_endian_short(bufp, outPtr1[i]);
               bufp += bytesPerSamp;
            }
         }
         else {  /* nChans = 2 */
            for (i = 0; i < dataArraySize; i++) {
               m_set_big_endian_short(bufp, outPtr1[i]);
               bufp += bytesPerSamp;
               m_set_big_endian_short(bufp, outPtr2[i]);
               bufp += bytesPerSamp;
            }
         }
         break;
      case MUS_LSHORT:
         if (nChans == 1) {
            for (i = 0; i < dataArraySize; i++) {
               m_set_little_endian_short(bufp, outPtr1[i]);
               bufp += bytesPerSamp;
            }
         }
         else {  /* nChans = 2 */
            for (i = 0; i < dataArraySize; i++) {
               m_set_little_endian_short(bufp, outPtr1[i]);
               bufp += bytesPerSamp;
               m_set_little_endian_short(bufp, outPtr2[i]);
               bufp += bytesPerSamp;
            }
         }
         break;
      default:
         fprintf(stderr, "writeData: unknown output data format!\n");
         exit(1);
         break;
   }

   nBytes = dataArraySize * nChans * bytesPerSamp;

   if (write(outfd, obuf, nBytes) == -1) {
      perror("writeData (write)");
      exit(1);
   }

   return 0;
}


#ifdef DEBUG
static int pof = 0;             /* positive overflow count */
static int nof = 0;             /* negative overflow count */
#endif

static INLINE HWORD
WordToHword(WORD v, int scl)
{
   HWORD out;
   WORD llsb = (1 << (scl - 1));
   v += llsb;                   /* round */
   v >>= scl;
   if (v > MAX_HWORD) {
#ifdef DEBUG
      if (pof == 0)
         fprintf(stderr, "*** resample: sound sample overflow\n");
      else if ((pof % 10000) == 0)
         fprintf(stderr, "*** resample: another ten thousand overflows\n");
      pof++;
#endif
      v = MAX_HWORD;
   }
   else if (v < MIN_HWORD) {
#ifdef DEBUG
      if (nof == 0)
         fprintf(stderr, "*** resample: sound sample (-) overflow\n");
      else if ((nof % 1000) == 0)
         fprintf(stderr, "*** resample: another thousand (-) overflows\n");
      nof++;
#endif
      v = MIN_HWORD;
   }
   out = (HWORD) v;
   return out;
}


/* Sampling rate conversion using linear interpolation for maximum speed.
 */
static int
SrcLinear(HWORD X[], HWORD Y[], double factor, UWORD *Time, UHWORD Nx)
{
   HWORD iconst;
   HWORD *Xp, *Ystart;
   WORD v, x1, x2;

   double dt;                        /* Step through input signal */
   UWORD dtb;                        /* Fixed-point version of Dt */
   UWORD endTime;                    /* When Time reaches EndTime, return */

   dt = 1.0 / factor;                /* Output sampling period */
   dtb = dt * (1 << Np) + 0.5;       /* Fixed-point representation */

   Ystart = Y;
   endTime = *Time + (1 << Np) * (WORD) Nx;
   while (*Time < endTime) {
      iconst = (*Time) & Pmask;
      Xp = &X[(*Time) >> Np];        /* Ptr to current input sample */
      x1 = *Xp++;
      x2 = *Xp;
      x1 *= ((1 << Np) - iconst);
      x2 *= iconst;
      v = x1 + x2;
      *Y++ = WordToHword(v, Np);     /* Deposit output */
      *Time += dtb;                  /* Move to next sample by time increment */
   }
   return (Y - Ystart);              /* Return number of output samples */
}


/* Sampling rate up-conversion only subroutine;
 * Slightly faster than down-conversion;
 */
static int
SrcUp(HWORD X[], HWORD Y[], double factor, UWORD *Time, UHWORD Nx,
      UHWORD Nwing, UHWORD LpScl, HWORD Imp[], HWORD ImpD[], BOOL Interp)
{
   HWORD *Xp, *Ystart;
   WORD v;

   double dt;                        /* Step through input signal */
   UWORD dtb;                        /* Fixed-point version of Dt */
   UWORD endTime;                    /* When Time reaches EndTime, return */

   dt = 1.0 / factor;                /* Output sampling period */
   dtb = dt * (1 << Np) + 0.5;       /* Fixed-point representation */

   Ystart = Y;
   endTime = *Time + (1 << Np) * (WORD) Nx;
   while (*Time < endTime) {
      Xp = &X[*Time >> Np];          /* Ptr to current input sample */
                                     /* Perform left-wing inner product: */
      v = FilterUp(Imp, ImpD, Nwing, Interp, Xp, (HWORD)(*Time & Pmask), -1);
                                     /* Perform right-wing inner product: */
      v += FilterUp(Imp, ImpD, Nwing, Interp, Xp + 1,
                                                 (HWORD)((-*Time) & Pmask), 1);
      v >>= Nhg;                     /* Make guard bits */
      v *= LpScl;                    /* Normalize for unity filter gain */
      *Y++ = WordToHword(v, NLpScl); /* strip guard bits, deposit output */
      *Time += dtb;                  /* Move to next sample by time increment */
   }
   return (Y - Ystart);              /* Return the number of output samples */
}


/* Sampling rate conversion subroutine */

static int
SrcUD(HWORD X[], HWORD Y[], double factor, UWORD *Time, UHWORD Nx,
      UHWORD Nwing, UHWORD LpScl, HWORD Imp[], HWORD ImpD[], BOOL Interp)
{
   HWORD *Xp, *Ystart;
   WORD v;

   double dh;                        /* Step through filter impulse response */
   double dt;                        /* Step through input signal */
   UWORD endTime;                    /* When Time reaches EndTime, return */
   UWORD dhb, dtb;                   /* Fixed-point versions of Dh,Dt */

   dt = 1.0 / factor;                /* Output sampling period */
   dtb = dt * (1 << Np) + 0.5;       /* Fixed-point representation */

   dh = MIN(Npc, factor * Npc);      /* Filter sampling period */
   dhb = dh * (1 << Na) + 0.5;       /* Fixed-point representation */

   Ystart = Y;
   endTime = *Time + (1 << Np) * (WORD) Nx;
   while (*Time < endTime) {
      Xp = &X[*Time >> Np];          /* Ptr to current input sample */
                                     /* Perform left-wing inner product: */
      v = FilterUD(Imp, ImpD, Nwing, Interp, Xp,
                                            (HWORD)(*Time & Pmask), -1, dhb);
                                     /* Perform right-wing inner product: */
      v += FilterUD(Imp, ImpD, Nwing, Interp, Xp + 1,
                                            (HWORD)((-*Time) & Pmask), 1, dhb);
      v >>= Nhg;                     /* Make guard bits */
      v *= LpScl;                    /* Normalize for unity filter gain */
      *Y++ = WordToHword(v, NLpScl); /* strip guard bits, deposit output */
      *Time += dtb;                  /* Move to next sample by time increment */
   }
   return (Y - Ystart);              /* Return the number of output samples */
}


static int
err_ret(char *s)
{
   fprintf(stderr, "resample: %s \n\n", s);  /* Display error message  */
   return -1;
}


/* returns number of output samples */
static int
resampleFast(double factor,        /* factor = Sndout/Sndin */
             int    infd,          /* input and output file descriptors */
             int    outfd,
             int    inDataFormat,  /* sndlib data format code */
             int    outDataFormat,
             int    inCount,       /* number of input samples to convert */
             int    outCount,      /* number of output samples to compute */
             int    nChans)        /* number of sound channels (1 or 2) */
{
   UWORD Time, Time2;              /* Current time/pos in input sample */
   UHWORD Xp, Ncreep, Xoff, Xread;
   int OBUFFSIZE = (int) (((double) IBUFFSIZE) * factor + 2.0);
   HWORD X1[IBUFFSIZE], Y1[OBUFFSIZE];      /* I/O buffers */
   HWORD X2[IBUFFSIZE], Y2[OBUFFSIZE];      /* I/O buffers */
   UHWORD Nout, Nx;
   int i, Ycount, last;

   Xoff = 10;

   Nx = IBUFFSIZE - 2 * Xoff;   /* # of samples to process each iteration */
   last = 0;                    /* Have not read last input sample yet */
   Ycount = 0;                  /* Current sample and length of output file */

   Xp = Xoff;                   /* Current "now"-sample pointer for input */
   Xread = Xoff;                /* Position in input array to read into */
   Time = (Xoff << Np);         /* Current-time pointer for converter */

   /* Need Xoff zeros at begining of sample */
   for (i = 0; i < Xoff; i++)
      X1[i] = X2[i] = 0;

   do {
      if (!last) {              /* If haven't read last sample yet */
         last = readData(infd, inDataFormat, inCount, X1, X2, IBUFFSIZE,
                                                      nChans, (int)Xread);
         if (last && (last - Xoff < Nx)) {  /* If last sample has been read, */
            Nx = last - Xoff;            /* calc last samp affected by filter */
            if (Nx <= 0)
               break;
         }
      }

      /* Resample stuff in input buffer */
      Time2 = Time;
      Nout = SrcLinear(X1, Y1, factor, &Time, Nx);
      if (nChans == 2)
         Nout = SrcLinear(X2, Y2, factor, &Time2, Nx);

      Time -= (Nx << Np);           /* Move converter Nx samples back in time */
      Xp += Nx;                     /* Advance by number of samples processed */
      Ncreep = (Time >> Np) - Xoff; /* Calc time accumulation in Time */
      if (Ncreep) {
         Time -= (Ncreep << Np);    /* Remove time accumulation */
         Xp += Ncreep;              /* and add it to read pointer */
      }

      /* Copy part of input signal that must be re-used */
      for (i = 0; i < IBUFFSIZE - Xp + Xoff; i++) {
         X1[i] = X1[i + Xp - Xoff];
         if (nChans == 2)
            X2[i] = X2[i + Xp - Xoff];
      }
      if (last) {                   /* If near end of sample... */
         last -= Xp;                /* ...keep track were it ends */
         if (!last)                 /* Lengthen input by 1 sample if... */
            last++;                 /* ...needed to keep flag TRUE */
      }
      Xread = i;                   /* Pos in input buff to read new data into */
      Xp = Xoff;

      Ycount += Nout;
      if (Ycount > outCount) {
         Nout -= (Ycount - outCount);
         Ycount = outCount;
      }

      if (Nout > OBUFFSIZE)        /* Check to see if output buff overflowed */
         return err_ret("Output array overflow");

      (void) writeData(outfd, outDataFormat, Y1, Y2, Nout, nChans);

      printf("."); fflush(stdout);

   } while (Ycount < outCount);    /* Continue until done */

   return (Ycount);                /* Return # of samples in output file */
}


/* number of output samples returned */
static int
resampleWithFilter(double factor,      /* factor = Sndout/Sndin */
                   int    infd,        /* input and output file descriptors */
                   int    outfd,
                   int    inDataFormat, /* sndlib data format code */
                   int    outDataFormat,
                   int    inCount,     /* number of input samples to convert */
                   int    outCount,    /* number of output samples to compute */
                   int    nChans,      /* number of sound channels (1 or 2) */
                   BOOL   interpFilt,  /* should interpolate filter coeffs? */
                   HWORD  Imp[],
                   HWORD  ImpD[],
                   UHWORD LpScl,
                   UHWORD Nmult,
                   UHWORD Nwing)
{
   UWORD Time, Time2;                  /* Current time/pos in input sample */
   UHWORD Xp, Ncreep, Xoff, Xread;
   int OBUFFSIZE = (int) (((double) IBUFFSIZE) * factor + 2.0);
   HWORD X1[IBUFFSIZE], Y1[OBUFFSIZE];  /* I/O buffers */
   HWORD X2[IBUFFSIZE], Y2[OBUFFSIZE];  /* I/O buffers */
   UHWORD Nout, Nx;
   int i, Ycount, last;

   /* Account for increased filter gain when using factors less than 1 */
   if (factor < 1)
      LpScl = LpScl * factor + 0.5;

   /* Calc reach of LP filter wing & give some creeping room */
   Xoff = ((Nmult + 1) / 2.0) * MAX(1.0, 1.0 / factor) + 10;

   /* Check input buffer size */
   if (IBUFFSIZE < 2 * Xoff)
      return err_ret("IBUFFSIZE (or factor) is too small");

   Nx = IBUFFSIZE - 2 * Xoff;   /* # of samples to process each iteration */

   last = 0;                    /* Have not read last input sample yet */
   Ycount = 0;                  /* Current sample and length of output file */
   Xp = Xoff;                   /* Current "now"-sample pointer for input */
   Xread = Xoff;                /* Position in input array to read into */
   Time = (Xoff << Np);         /* Current-time pointer for converter */

   /* Need Xoff zeros at begining of sample */
   for (i = 0; i < Xoff; i++)
      X1[i] = X2[i] = 0;

   do {
      if (!last) {                       /* If haven't read last sample yet */
         last = readData(infd, inDataFormat, inCount, X1, X2, IBUFFSIZE,
                                                      nChans, (int)Xread);
         if (last && (last - Xoff < Nx)) {  /* If last sample has been read, */
            Nx = last - Xoff;            /* calc last samp affected by filter */
            if (Nx <= 0)
               break;
         }
      }
      /* Resample stuff in input buffer */
      Time2 = Time;
      if (factor >= 1) {        /* SrcUp() is faster if we can use it */
         Nout = SrcUp(X1, Y1, factor, &Time, Nx, Nwing, LpScl, Imp, ImpD,
                      interpFilt);
         if (nChans == 2)
            Nout = SrcUp(X2, Y2, factor, &Time2, Nx, Nwing, LpScl, Imp, ImpD,
                         interpFilt);
      }
      else {
         Nout = SrcUD(X1, Y1, factor, &Time, Nx, Nwing, LpScl, Imp, ImpD,
                      interpFilt);
         if (nChans == 2)
            Nout = SrcUD(X2, Y2, factor, &Time2, Nx, Nwing, LpScl, Imp, ImpD,
                         interpFilt);
      }

      Time -= (Nx << Np);          /* Move converter Nx samples back in time */
      Xp += Nx;                    /* Advance by number of samples processed */
      Ncreep = (Time >> Np) - Xoff;  /* Calc time accumulation in Time */
      if (Ncreep) {
         Time -= (Ncreep << Np);   /* Remove time accumulation */
         Xp += Ncreep;             /* and add it to read pointer */
      }

      /* Copy part of input signal that must be re-used */
      for (i = 0; i < IBUFFSIZE - Xp + Xoff; i++) {
         X1[i] = X1[i + Xp - Xoff];
         if (nChans == 2)
            X2[i] = X2[i + Xp - Xoff];
      }
      if (last) {                  /* If near end of sample... */
         last -= Xp;               /* ...keep track were it ends */
         if (!last)                /* Lengthen input by 1 sample if... */
            last++;                /* ...needed to keep flag TRUE */
      }
      Xread = i;                   /* Pos in input buff to read new data into */
      Xp = Xoff;

      Ycount += Nout;
      if (Ycount > outCount) {
         Nout -= (Ycount - outCount);
         Ycount = outCount;
      }

      if (Nout > OBUFFSIZE)        /* Check to see if output buff overflowed */
         return err_ret("Output array overflow");

      (void) writeData(outfd, outDataFormat, Y1, Y2, Nout, nChans);

      printf("."); fflush(stdout);

   } while (Ycount < outCount);    /* Continue until done */

   return (Ycount);                /* Return # of samples in output file */
}


/* returns number of output samples */
int
resample(double factor,         /* factor = Sndout/Sndin */
         int    infd,           /* input and output file descriptors */
         int    outfd,
         int    inDataFormat,   /* sndlib data format code */
         int    outDataFormat,
         int    inCount,        /* number of input samples to convert */
         int    outCount,       /* number of output samples to compute */
         int    nChans,         /* number of sound channels (1 or 2) */
         BOOL   interpFilt,     /* TRUE means interpolate filter coeffs */
         int    fastMode,       /* 0 = highest quality, slowest speed */
         BOOL   largeFilter,    /* TRUE means use 65-tap FIR filter */
         FiltSpec *filtspec)    /* NULL for internal filter, else makeFilter */
{
   UHWORD LpScl;                /* Unity-gain scale factor */
   UHWORD Nwing;                /* Filter table size */
   UHWORD Nmult;                /* Filter length for up-conversions */
   HWORD *Imp = 0;              /* Filter coefficients */
   HWORD *ImpD = 0;             /* ImpD[n] = Imp[n+1]-Imp[n] */

   if (fastMode)
      return resampleFast(factor, infd, outfd, inDataFormat, outDataFormat,
                                                inCount, outCount, nChans);

#ifdef DEBUG
   /* Check for illegal constants */
   if (Np >= 16)
      return err_ret("Error: Np>=16");
   if (Nb + Nhg + NLpScl >= 32)
      return err_ret("Error: Nb+Nhg+NLpScl>=32");
   if (Nh + Nb > 32)
      return err_ret("Error: Nh+Nb>32");
#endif

   /* Set defaults */

   if (filtspec) {
      int err;

      Imp = (HWORD *)calloc(MAXNWING, sizeof(HWORD));
      ImpD = (HWORD *)calloc(MAXNWING, sizeof(HWORD));
      if (Imp == NULL || ImpD == NULL) {
         fprintf(stderr, "Can't allocate filter tables.\n");
         exit(1);
      }
      Nmult = filtspec->length;
      Nwing = Npc * (Nmult - 1) / 2;  /* num. of filter coeffs in right wing */
      err = makeFilter(Imp, ImpD, &LpScl, Nwing,
                                          filtspec->rolloff, filtspec->beta);
      if (err == 4) {
         fprintf(stderr, "Unity-gain scale factor overflows 16-bit "
                         "half-word.\nFilter design was probably way off. "
                         "Try relaxing specs.\n");
         exit(1);
      }
      else if (err) {          /* should've caught this already */
         fprintf(stderr, "Internal program error.\n");
         exit(1);
      }
   }
   else if (largeFilter) {
      Nmult = LARGE_FILTER_NMULT;
      Imp = LARGE_FILTER_IMP;   /* Impulse response */
      ImpD = LARGE_FILTER_IMPD; /* Impulse response deltas */
      LpScl = LARGE_FILTER_SCALE;  /* Unity-gain scale factor */
      Nwing = LARGE_FILTER_NWING;  /* Filter table length */
   }
   else {
      Nmult = SMALL_FILTER_NMULT;
      Imp = SMALL_FILTER_IMP;   /* Impulse response */
      ImpD = SMALL_FILTER_IMPD; /* Impulse response deltas */
      LpScl = SMALL_FILTER_SCALE;  /* Unity-gain scale factor */
      Nwing = SMALL_FILTER_NWING;  /* Filter table length */
   }
#ifdef ATTENUATE_SCALE_FACTOR
 #if DEBUG
   fprintf(stderr, "Attenuating resampler scale factor by 0.95 "
                   "to reduce probability of clipping\n");
 #endif
   LpScl *= 0.95;
#endif
   return resampleWithFilter(factor, infd, outfd, inDataFormat,
                             outDataFormat, inCount, outCount, nChans,
                             interpFilt, Imp, ImpD, LpScl, Nmult, Nwing);
}

