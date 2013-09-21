// RVB.C --	Post-processing reverb.  Takes dry signal from normal input and
// 			summed first and second generation reflections from a global array.

#include "RVB.h"
#include "common.h"
#include <rtdefs.h>
#include <ugens.h>
#include <string.h>
#include <stdio.h>
#ifndef MAXMSP
#include <pthread.h>
#endif
#include "msetup.h"

//#define debug
//#define SIG_DEBUG
//#define LOOP_DEBUG
//#define DELAY_DEBUG

#ifdef SIG_DEBUG
#define DBG(stmt) { stmt; }
#else
#define DBG(stmt)
#endif

#if defined(SIG_DEBUG) || defined(LOOP_DEBUG)
#define DBG1(stmt) { stmt; }
#else
#define DBG1(stmt)
#endif

static inline void PrintInput(float *sig, int len)
{
    for (int i = 0; i < len; i++)
	    if (sig[i] != 0.0)
		    printf("sig[%d] = %f\n", i, sig[i]);
	printf("\n");
}

static inline void PrintSig(double *sig, int len, double threshold = 0.0)
{
    for (int i = 0; i < len; i++)
	    if (sig[i] > threshold || sig[i] < -threshold)
		    printf("sig[%d] = %f\n", i, sig[i]);
	printf("\n");
}

extern "C" {
   #include <cmixfuns.h>
}

int RVB::primes[NPRIMES + 2];
AtomicInt RVB::primes_gotten = -1;

/* ------------------------------------------------------------ makeRVB --- */
Instrument *makeRVB()
{
   RVB *inst;

   inst = new RVB();
   inst->set_bus_config("RVB");

   return inst;
}

RVB::RVB()
{
	in = NULL;
	_branch = 0;
    for (int i = 0; i < 2; i++) {
       int j;
       for (j = 0; j < 6; j++)
          m_rvbData[i][j].Rvb_del = NULL;
	}
    get_primes(NPRIMES, primes);        /* load array with prime numbers */
}

RVB::~RVB()
{
	delete [] in;
	// Dont delete memory for global arrays if insts are still active
	if (check_users() == 0) {
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 6; j++)
				delete [] m_rvbData[i][j].Rvb_del;
		}
	}
}

int RVB::init(double p[], int n_args)
{
    float  outskip, inskip, rvb_time;

    outskip = p[0];
    inskip = p[1];
    m_dur = p[2];
    if (m_dur < 0)                      /* "dur" represents timend */
        m_dur = -m_dur - inskip;

	if (rtsetinput(inskip, this) == -1) { // no input
	  return(DONT_SCHEDULE);
	}

    insamps = (int)(m_dur * SR);
    m_amp = p[3];

    if (inputChannels() != 4)
		return die(name(), "Input must be 4-channel (2 sig+early refl, 2 reverb input).");
	
	if (outputChannels() != 2)
		return die(name(), "Output must be stereo.");

    double Matrix[12][12];
   
    /* Get results of Minc setup calls (space, mikes_on, mikes_off, matrix) */
    if (get_rvb_setup_params(Dimensions, Matrix, &rvb_time) == -1)
       return die(name(), "You must call setup routine `space' first.");
    /* (perform some initialization that used to be in space.c) */
    int meanLength = MFP_samps(SR, Dimensions); // mean delay length for reverb
    get_lengths(meanLength);              /* sets up delay lengths */
    set_gains(rvb_time);                		/* sets gains for filters */
	set_random();                       /* sets up random variation of delays */
    set_allpass();
   
	wire_matrix(Matrix);
	
	if (rtsetoutput(outskip, m_dur + rvb_time, this) == -1)
		return DONT_SCHEDULE;
	DBG1(printf("nsamps = %d\n", nSamps()));
	return nSamps();
}

int RVB::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];

    alloc_delays();                     /* allocates memory for delays */
	rvb_reset();
	return 0;
}

/* ------------------------------------------------------------------ run --- */
int RVB::run()
{
	double rvbsig[2][8192];
	const int frames = framesToRun();
	const int inChans = inputChannels();

    rtgetin(in, this, frames * inChans);

	register float *outptr = &this->outbuf[0];
	/* run summed 1st and 2nd generation paths through reverberator */
 	for (int n = 0; n < frames; n++) {
		if (--_branch <= 0) {
			double p[4];
			update(p, 4);
			m_amp = p[3];
			_branch = getSkip();
		}
		if (m_amp != 0.0) {
			double rmPair[2];
			double rvbPair[2];
			rmPair[0] = in[n*inChans+2];
			rmPair[1] = in[n*inChans+3];
			doRun(rmPair, rvbPair, currentFrame() + n);
			rvbsig[0][n] = rvbPair[0] * m_amp;
			rvbsig[1][n] = rvbPair[1] * m_amp;
		}
		else
			rvbsig[0][n] = rvbsig[1][n] = 0.0;
		/* sum the input signal (which includes early response) & reverbed sigs  */
		*outptr++ = in[n*inChans] + rvbsig[0][n];
		*outptr++ = in[n*inChans+1] + rvbsig[1][n];
	}
	increment(frames);
		
	DBG(printf("FINAL MIX:\n"));
	DBG(PrintInput(&this->outbuf[0], RTBUFSAMPS));
	DBG(PrintInput(&this->outbuf[1], RTBUFSAMPS));
	
	return frames;
}

/* -------------------------------------------------------------- delpipe --- */
/* delpipe is a delay with a variable interpolated tap, used by reverb. */

inline double
delpipe(double sig, int *counter, double nsdel, int delsize, double *delay)
{
   register int intap, tap1, tap2, del1;

   intap = *counter;
   if (intap >= delsize)
       intap -= delsize;
   delay[intap] = sig;                    /* put samp into delay */
   del1 = (int)nsdel;
   tap1 = intap - del1;
   if (tap1 < 0)
       tap1 += delsize;
   tap2 = tap1 - 1;
   if (tap2 < 0)
       tap2 += delsize;
   
   *counter = ++intap;			// increment and store
  
   const double frac = nsdel - del1;
   return (delay[tap1] + (delay[tap2] - delay[tap1]) * frac);
}

/* -------------------------------------------------------------- Allpass --- */
/* Allpass is the allpass filter used by reverb */
inline double
Allpass(double sig, int *counter, double *data)
{
   register int nsdel, intap, length, outtap;
   double *delay = &data[2];

   nsdel = (int)data[1];
   length = nsdel + 1;
   intap = *counter;
   if (intap >= length)
       intap -= length;
   outtap = intap - nsdel;
   if (outtap < 0)
      outtap += length;

   /* Input = sig + gain * (sig - delayed out)  */

   double gsig = data[0] * (sig - delay[outtap]);
   delay[intap] = sig + gsig;

   *counter = ++intap;	// increment and store
   
   /* Output = delayed out + gain * (sig - delayed out)  */
   return (delay[outtap] + gsig);
}

// define USE_BUGGY_CODE to recreate buggy code which produces decent reverb -- correct code does not!

#undef USE_HI_PASS

/* ------------------------------------------------------------------ RVB --- */
/* This is the main routine, doRun. It is a bank of 2x6 randomly varying
   delays which feed back into themselves in a tricky way.
*/
void
RVB::doRun(double *input, double *output, long counter)
{
   register int i, j;
   double sig, delsig;

   for (i = 0; i < 2; ++i) {                /* loop for 2 output chans */
      output[i] = 0.0;
      for (j = 0; j < 6; ++j) {             /* loop for 6 delays per chan */
		 ReverbData *rvb = &m_rvbData[i][j];
	 	 ReverbData *rvb2 = rvb;
	 
         sig = input[i] + rvb->delin;      /* combine input w/ delay return */

         /* get new delay length (random) then 
            put samp into delay & get new delayed samp out
         */
         double delay = Nsdelay[i][j] + randi(&rvb->Rand_info[0]);
         delsig = delpipe(sig, &rvb->deltap, delay, rvbdelsize, &rvb->Rvb_del[0]);

#ifdef USE_BUGGY_CODE
         if (i == 1 && j > 0)
	     rvb2 = &m_rvbData[0][j];	// remap "incorrectly"
#endif
         /* filter with air simulation filters, set gains */
         rvb->delout = tone(delsig, &rvb2->Rvb_air[0]);

         /* sum outputs of all delays for 2 channels */
         output[i] += rvb->delout;
      }
      /* run outputs through Allpass filter */
	  double sigout = Allpass(output[i], &allpassTap[i], Allpass_del[i]);
#ifdef USE_HI_PASS
	  output[i] = 0.51 * sigout - 0.49 * m_rvbPast[i];	// Hi pass FIR filter
      m_rvbPast[i] = sigout;
#else
	  output[i] = sigout;
#endif
   }

   /* redistribute delpipe outs into delpipe ins */
   matrix_mix();
}

/* ----------------------------------------------------------- matrix_mix --- */
/* matrix_mix distributes the outputs from the delays (in reverb) back
   into the inputs according to the signal routing matrix set up by the
   matrix routine.
*/
void
RVB::matrix_mix()
{
	for (int i = 0; i < 12; ++i) {
		ReverbPatch *patch = &ReverbPatches[i];
		double out = 0.0;
		for (int n = 0; n < patch->incount; n++) {
			out += *patch->inptrs[n] * patch->gains[n];
		}
   		*patch->outptr = out;
	}
}	        

/* ----------------------------------------------------------- set_random --- */
/* Initializes the random functions used to vary the delay lengths
   in the reverb routine.
*/
void
RVB::set_random()
{
   static const float rnd_freq[2][6] = {    /* the freq of rand variations */
      {4.9, 5.2, 6.7, 3.4, 2.7, 3.8},
      {5.1, 6.3, 7.0, 3.6, 2.8, 4.4}
   };
   static const float rnd_pcnt[2][6] = {    /* the amt. of delay to vary */
      {.0016, .0013, .0021, .0018, .0019, .0014},
      {.0027, .0014, .0020, .0016, .0012, .0015}
   };
   static const float seed[2][6] = {        /* the seeds for randi */
      {.314, .159, .265, .358, .979, .323},
      {.142, .857, .685, .246, .776, .456}
   };

   for (int i = 0; i < 2; i++) {              /* loop for 2 chans worth */
      for (int j = 0; j < 6; j++) {           /* loop for 6 delays per chan */
         ReverbData *r = &m_rvbData[i][j];
         r->Rand_info[0] = rnd_pcnt[i][j] * SR / 2.0;    /* nsamps jitter */
         r->Rand_info[1] = cycle(SR, rnd_freq[i][j], 512);   /* SI  */
         r->Rand_info[2] = 1.0;
         r->Rand_info[4] = seed[i][j];     /* load seeds */
      }
   }
}

/* ---------------------------------------------------------- get_lengths --- */
/* Computes the lengths for the 2x6 delays used in reverb, and determines
   the max number of elements needed for allocation.
*/
void
RVB::get_lengths(long m_length)
{
   int i, j, max = 0;
   static float delfac[2][6] = {        /* These determine the distribution */
      {0.819, 0.918, 1.0, 1.11, 1.18, 1.279},
      {0.811, 0.933, 0.987, 1.13, 1.21, 1.247}
   };

   for (i = 0; i < 2; ++i) {
      for (j = 0; j < 6; ++j) {
         int val = (int)(m_length * delfac[i][j] + 0.5); /* 2x6 diff. lengths */
         int pval = close_prime(val, NPRIMES, primes);  /* all lengths primes */
         Nsdelay[i][j] = pval;

         /* the number of elements (max) for allocation */
         max = (pval > max) ? pval : max;
      }
   }
   /* extra to allow for varying taps dur to random tap */
   rvbdelsize = (int)(max * 1.2);

}

/* ------------------------------------------------------------ set_gains --- */
/* Determines the proper gains for the feedback filters in reverb, based on
   reverb time and average delay length. It also creates the global array of
   coefficients.
*/
void 
RVB::set_gains(float rvbtime)
{
   int    i, fpoint, nvals = 16;
   float  rescale, gain, dist, G1, temp = SR / MACH1;
   double adjust;
   static float array[16] = {
      0, .001, 10, .1, 25, .225, 35, .28, 50, .35, 65, .4, 85, .45, 95, .475
   };

   /* compensate for variable delay lengths */
   rescale = (0.05 * SR) / Nsdelay[0][0];

   /* compensate for SR differences */
   adjust = 1.0 - (0.42 * (SR - 25000) / 25000.0);

   /* create scaled curve for coeffs */
   setline(array, nvals, NCOEFFS, AIRCOEFFS);

   for (i = 0; i < NCOEFFS; i++)
      AIRCOEFFS[i] = pow((double)AIRCOEFFS[i], adjust);

   gain = 1.0 - 0.366 / (rvbtime * rescale);               /* a la Moorer */
   gain = (gain < 0.0) ? 0.0 : gain;
#ifdef debug
   printf("number of samples in each reverb delay line.");
#endif
   for (i = 0; i < 2; ++i) {
#ifdef debug
      printf("\nchannel %d:\n\n", i);
#endif
      for (int j = 0; j < 6; ++j) {
         dist = Nsdelay[i][j] / temp;
         fpoint = (int)(dist * (float)NCOEFFS / 300.0); /* 300 ft. max dist. */
         /* to avoid overrunning: */
         fpoint = (fpoint <= NCOEFFS - 1) ? fpoint : NCOEFFS - 1;
         G1 = AIRCOEFFS[fpoint - 1];                 /* G1 is filter coeff. */
         m_rvbData[i][j].Rvb_air[0] = gain * (1.0 - G1);       /* G2 is filt. gain */
         m_rvbData[i][j].Rvb_air[1] = G1;
#ifdef debug
         printf("delay %d: %g (%7.2f ms.) g1 = %f\n\n", j, Nsdelay[i][j],
                float(Nsdelay[i][j] * 1000.0 / SR), G1);
#endif
      }
   }
}

/* --------------------------------------------------------- alloc_delays --- */
/* Sets aside the memory needed for the delays in RVB
*/
void 
RVB::alloc_delays()
{
   /* allocate memory for reverb delay lines */
   for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 6; j++) {
         m_rvbData[i][j].Rvb_del = new double[rvbdelsize];
         memset(m_rvbData[i][j].Rvb_del, 0, sizeof(double) * rvbdelsize);
      }
   }
}

/* ---------------------------------------------------------- set_allpass --- */
/* Initializes the allpass filters for reverb.
*/
void 
RVB::set_allpass()
{
   Allpass_del[0][0] = 0.72;         /* the gains */
   Allpass_del[1][0] = 0.72;
   Allpass_del[0][1] = 180.0;        /* the delay lengths */
   Allpass_del[1][1] = 183.0;
}

void
RVB::wire_matrix(double Matrix[12][12])
{
	int i, j, n;
	int row, col;
		
	// assign output pointers
	for (i = 0, n = 0; i < 2; i++)
		for (j = 0; j < 6; j++) {
		    ReverbPatches[n].outptr = &m_rvbData[i][j].delin;
			ReverbPatches[n].incount = 0;
			n++;
		}

	for (row = 0; row < 12; row++) {
	    for (col = 0, n = 0; col < 12; col++) {
			if (Matrix[row][col] != 0.0) {
				i = col / 6;
				j = col % 6;
				ReverbPatches[row].inptrs[ReverbPatches[row].incount] = &m_rvbData[i][j].delout;
				ReverbPatches[row].gains[ReverbPatches[row].incount++] = Matrix[row][col];
			}
		}
	}
}

/* ------------------------------------------------------------ rvb_reset --- */
/* reset zeroes out all delay and filter histories in reverb
*/
void 
RVB::rvb_reset()
{
	register int i, j, k;

	/* reset reverb filters and delays */

	for (i = 0; i < 2; ++i) {
		for (j = 0, k = 0; j < 6; ++j) {
			ReverbData *r = &m_rvbData[i][j];
			r->delin = r->delout = 0.0;
			r->deltap = 0;	// index for each delay unit
			r->Rvb_air[2] = 0.0;
			register double *point = r->Rvb_del;
			while (k < rvbdelsize)
				point[k++] = 0.0;
		}
		for (j = 2; j < 502; ++j)
			Allpass_del[i][j] = 0.0;
		allpassTap[i] = 0; 	// indices for allpass delay
		m_rvbPast[i] = 0.0;
	}
}

/* ----------------------------------------------------------- get_primes --- */
/* Loads <p> array with up to <x> prime numbers for use in determining
   prime delay lengths.
*/
void
RVB::get_primes(int x, int p[])
{
   int val = 5, index = 2;

#ifndef MAXMSP
	if (primes_gotten.incrementAndTest())
#else
	if (++primes_gotten == 0)
#endif
	{
	/* first 2 vals initialized */
	p[0] = 2;
	p[1] = 3;

	while (index < x) {
	   int flag = 1;
	   for (int i = 1; flag && val / p[i] >= p[i]; ++i)
              if (val % p[i] == 0)
        	 flag = 0;
	   if (flag) {
              p[index] = val;
              ++index;
	   }
	   val += 2;
	}
    }
}
