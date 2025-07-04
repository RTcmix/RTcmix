//
// Created by Douglas Scott on 4/5/25.
//

#include "RVBBASE.h"
#include "../MOVE/common.h"
#include <ugens.h>
#include <string.h>
#include <stdio.h>
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

template<int OutChannels, int DelayCount>
int RVBBASE<OutChannels, DelayCount>::primes[NPRIMES + 2];

template<int OutChannels, int DelayCount>
AtomicInt RVBBASE<OutChannels, DelayCount>::primes_gotten(-1);

template<int OutChannels, int DelayCount>
RVBBASE<OutChannels, DelayCount>::RVBBASE()
{
    in = NULL;
    _branch = 0;
    for (int i = 0; i < OutChannels; i++) {
        int j;
        for (j = 0; j < DelayCount; j++)
            m_rvbData[i][j].Rvb_del = NULL;
    }
    get_primes(NPRIMES, primes);        /* load array with prime numbers */
}

template<int OutChannels, int DelayCount>
RVBBASE<OutChannels, DelayCount>::~RVBBASE()
{
    delete [] in;
    // Dont delete memory for global arrays if insts are still active
    if (check_users() == 0) {
        uninit();
    }
}

template<int OutChannels, int DelayCount>
int RVBBASE<OutChannels, DelayCount>::init(double p[], int n_args) {
    double outskip, inskip, rvb_time;

    outskip = p[0];
    inskip = p[1];
    m_dur = p[2];
    if (m_dur < 0) {                     /* "dur" represents timend */
        m_dur = -m_dur - inskip;
    }
    if (rtsetinput((float)inskip, this) == -1) { // no input
        return(DONT_SCHEDULE);
    }

    insamps = (int)(m_dur * SR);
    m_amp = p[3];

    const int RequiredInputChannels = OutChannels * 2;      // One set for direct signal, one for reverb feed

    if (inputChannels() != RequiredInputChannels)
        return die(name(), "Input must be %d-channel (%d sig+early refl, %d reverb input).", RequiredInputChannels, RequiredInputChannels/2, RequiredInputChannels/2);

    if (outputChannels() != OutChannels)
        return die(name(), "Output must be %d-channel.", OutChannels);

    double Matrix[12][12];

    /* Get results of Minc setup calls (space, mikes_on, mikes_off, matrix) */
    if (get_rvb_setup_params(Dimensions, Matrix, &rvb_time) == -1)
        return die(name(), "You must call setup routine `space' first.");
    /* (perform some initialization that used to be in space.c) */
    long meanLength = MFP_samps(SR, Dimensions); // mean delay length for reverb
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

template<int OutChannels, int DelayCount>
int RVBBASE<OutChannels, DelayCount>::configure()
{
    try {
        in = new float[RTBUFSAMPS * inputChannels()];
        alloc_delays();                     /* allocates memory for delays */
        rvb_reset();
    }
    catch (...) {
        return DONT_SCHEDULE;       // memory allocation failure
    }
    return 0;
}

template<int OutChannels, int DelayCount>
void RVBBASE<OutChannels, DelayCount>::uninit()
{
    for (int i = 0; i < OutChannels; i++) {
        for (int j = 0; j < DelayCount; j++) {
            delete [] m_rvbData[i][j].Rvb_del;
            m_rvbData[i][j].Rvb_del = NULL;
        }
    }
}

/* ------------------------------------------------------------------ run --- */
template<int OutChannels, int DelayCount>
int RVBBASE<OutChannels, DelayCount>::run()
{
    double rvbsig[OutChannels][8192];
    const int frames = framesToRun();
    const int inChans = inputChannels();

    rtgetin(in, this, frames * inChans);
    DBG(printf("INPUT SIGNAL:\n"));
    DBG(PrintInput(&in[0], frames * inChans));
    float *outptr = &this->outbuf[0];
    /* run summed 1st and 2nd generation paths through reverberator */
    for (int n = 0; n < frames; n++) {
        if (--_branch <= 0) {
            double p[4];
            update(p, 4);
            m_amp = p[3];
            _branch = getSkip();
        }
        if (m_amp != 0.0) {
            double roomFrame[OutChannels];
            double rvbFrame[OutChannels];
            // read early response from interleaved input.
            // For mono-in, quad, the first frame will be in[4, 5, 6, 7].
            for (int ch = 0; ch < OutChannels; ++ch) {
                roomFrame[ch] = in[(n * inChans) + (ch + OutChannels)];
            }
            doRun(roomFrame, rvbFrame, currentFrame() + n);
            // Copy scaled reverb output into rvbsig.
            for (int ch = 0; ch < OutChannels; ++ch) {
                rvbsig[ch][n] = rvbFrame[ch] * m_amp;
            }
        }
        else {
            for (int ch = 0; ch < OutChannels; ++ch) {
                rvbsig[ch][n] = 0.0;
            }
        }
        /* sum the input signal (which includes early response) & reverbed sigs.
         * For mono-in, quad, the first input frame will be in[0, 1, 2, 3].
         */
        for (int ch = 0; ch < OutChannels; ++ch) {
            outptr[ch] = in[(n * inChans) + ch] + rvbsig[ch][n];
        }
        outptr += OutChannels;
    }
    increment(frames);

    DBG(printf("FINAL INTERLEAVED QUAD MIX:\n"));
    DBG(PrintInput(&this->outbuf[0], RTBUFSAMPS*OutChannels));

    return frames;
}

/* -------------------------------------------------------------- delpipe --- */
/* delpipe is a delay with a variable interpolated tap, used by reverb. */

inline double
delpipe(double sig, int *counter, double nsdel, int delsize, double *delay)
{
    int intap, tap1, tap2, del1;

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
    int nsdel, intap, length, outtap;
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
/* This is the main routine, doRun. It is a bank of OutChannelsxDelayCount randomly varying
   delays which feed back into themselves in a tricky way.
*/
template<int OutChannels, int DelayCount>
void
RVBBASE<OutChannels, DelayCount>::doRun(double *input, double *output, long counter)
{
    int i, j;
    double sig, delsig;

    for (i = 0; i < OutChannels; ++i) {                /* loop for output chans */
        output[i] = 0.0;
        for (j = 0; j < DelayCount; ++j) {             /* loop for 6 delays per chan */
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
template<int OutChannels, int DelayCount>
void
RVBBASE<OutChannels, DelayCount>::matrix_mix()
{
    for (int i = 0; i < OutChannels*DelayCount; ++i) {
        ReverbPatch *patch = &ReverbPatches[i];
        double out = 0.0;
        for (int n = 0; n < patch->incount; n++) {
            out += *patch->inptrs[n] * patch->gains[n];
        }
        *patch->outptr = out;
    }
}

/* ------------------------------------------------------------ set_gains --- */
/* Determines the proper gains for the feedback filters in reverb, based on
   reverb time and average delay length. It also creates the global array of
   coefficients.
*/
template<int OutChannels, int DelayCount>
void
RVBBASE<OutChannels, DelayCount>::set_gains(float rvbtime)
{
    int    i, fpoint, nvals = 16;
    float  temp = SR / MACH1;
    static double array[16] = {
            0, .001, 10, .1, 25, .225, 35, .28, 50, .35, 65, .4, 85, .45, 95, .475
    };

    /* compensate for variable delay lengths */
    double rescale = (0.05 * SR) / Nsdelay[0][0];

    /* compensate for SR differences */
    double adjust = 1.0 - (0.42 * (SR - 25000) / 25000.0);

    /* create scaled curve for coeffs */
    setline(array, nvals, NCOEFFS, AIRCOEFFS);

    for (i = 0; i < NCOEFFS; i++)
        AIRCOEFFS[i] = pow((double)AIRCOEFFS[i], adjust);

    double gain = 1.0 - 0.366 / (rvbtime * rescale);               /* a la Moorer */
    gain = (gain < 0.0) ? 0.0 : gain;
#ifdef debug
    printf("filter gain for rvb time %f seconds: %f\n", rvbtime, gain);
    printf("number of samples in each reverb delay line.");
#endif
    for (i = 0; i < OutChannels; ++i) {
#ifdef debug
        printf("\nchannel %d:\n\n", i);
#endif
        for (int j = 0; j < DelayCount; ++j) {
            double dist = Nsdelay[i][j] / temp;
            fpoint = (int)(dist * (float)NCOEFFS / 300.0); /* 300 ft. max dist. */
            /* to avoid overrunning: */
            fpoint = (fpoint <= NCOEFFS - 1) ? fpoint : NCOEFFS - 1;
            double G1 = AIRCOEFFS[fpoint - 1];                 /* G1 is filter coeff. */
            m_rvbData[i][j].Rvb_air[0] = gain * (1.0 - G1);       /* G2 is filt. gain */
            m_rvbData[i][j].Rvb_air[1] = G1;
#ifdef debug
            printf("delay %d: %g (%7.2f ms.) G1 = %f\n\n", j, Nsdelay[i][j],
                float(Nsdelay[i][j] * 1000.0 / SR), G1);
#endif
        }
    }
}

/* --------------------------------------------------------- alloc_delays --- */
/* Sets aside the memory needed for the delays in RVB
*/
template<int OutChannels, int DelayCount>
void
RVBBASE<OutChannels, DelayCount>::alloc_delays()
{
    /* allocate memory for reverb delay lines */
    for (int i = 0; i < OutChannels; i++) {
        for (int j = 0; j < DelayCount; j++) {
            m_rvbData[i][j].Rvb_del = new double[rvbdelsize];
            memset(m_rvbData[i][j].Rvb_del, 0, sizeof(double) * rvbdelsize);
        }
    }
}

/* ---------------------------------------------------------- set_allpass --- */
/* Initializes the allpass filters for reverb.
*/
template<int OutChannels, int DelayCount>
void
RVBBASE<OutChannels, DelayCount>::set_allpass()
{
    Allpass_del[0][0] = 0.72;         /* the gains */
    Allpass_del[1][0] = 0.72;
    Allpass_del[0][1] = 180.0;        /* the delay lengths */
    Allpass_del[1][1] = 183.0;
}

template<int OutChannels, int DelayCount>
void
RVBBASE<OutChannels, DelayCount>::wire_matrix(double Matrix[12][12])
{
    int i, j, n;
    int row, col;

    // assign output pointers
    for (i = 0, n = 0; i < OutChannels; i++)
        for (j = 0; j < DelayCount; j++) {
            ReverbPatches[n].outptr = &m_rvbData[i][j].delin;
            ReverbPatches[n].incount = 0;
            n++;
        }

    for (row = 0; row < 12; row++) {
        for (col = 0, n = 0; col < 12; col++) {
            if (Matrix[row][col] != 0.0) {
                int chan = col / DelayCount;
                int path = col % DelayCount;
#ifdef debug
                printf("Matrix[row %d][col %d] -> rvb chan %d path %d\n", row, col, chan, path);
#endif
                ReverbPatches[row].inptrs[ReverbPatches[row].incount] = &m_rvbData[chan][path].delout;
                ReverbPatches[row].gains[ReverbPatches[row].incount++] = Matrix[row][col];
            }
        }
    }
}

/* ------------------------------------------------------------ rvb_reset --- */
/* reset zeroes out all delay and filter histories in reverb
*/
template<int OutChannels, int DelayCount>
void
RVBBASE<OutChannels, DelayCount>::rvb_reset()
{
    int i, j, k;

    /* reset reverb filters and delays */

    for (i = 0; i < OutChannels; ++i) {
        for (j = 0, k = 0; j < DelayCount; ++j) {
            ReverbData *r = &m_rvbData[i][j];
            r->delin = r->delout = 0.0;
            r->deltap = 0;	// index for each delay unit
            r->Rvb_air[2] = 0.0;
            double *point = r->Rvb_del;
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
template<int OutChannels, int DelayCount>
void
RVBBASE<OutChannels, DelayCount>::get_primes(int x, int p[])
{
    int val = 5, index = 2;

    if (primes_gotten.incrementAndTest())
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

template class RVBBASE<2, 6>;
template class RVBBASE<4, 3>;
