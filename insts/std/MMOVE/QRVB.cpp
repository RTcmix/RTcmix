//
// Created by Douglas Scott on 4/5/25.
//

#include "QRVB.h"
#include "../MOVE/common.h"

/* ------------------------------------------------------------ makeRVB --- */
Instrument *makeQRVB()
{
    QRVB *inst;

    inst = new QRVB();
    inst->set_bus_config("QRVB");

    return inst;
}

QRVB::QRVB() {}

QRVB::~QRVB() {}

/* ---------------------------------------------------------- get_lengths --- */
/* Computes the lengths for the OutChannelsxDelayCount delays used in reverb, and determines
   the max number of elements needed for allocation.
*/
void
QRVB::get_lengths(long m_length)
{
    int i, j, max = 0;
    static float delfac[QRVB_CHANS][QRVB_DELCOUNT] = {        /* These determine the distribution */
            {0.819, 1.0, 1.18 },
            {0.918, 1.11, 1.279},
            {0.811, 0.987, 1.21 },
            {0.933, 1.13, 1.247}
    };

    for (i = 0; i < QRVB_CHANS; ++i) {
        for (j = 0; j < QRVB_DELCOUNT; ++j) {
            int val = (int)(m_length * delfac[i][j] + 0.5); /* OutChannelsxDelayCount diff. lengths */
            int pval = close_prime(val, NPRIMES, primes);  /* all lengths primes */
            Nsdelay[i][j] = pval;

            /* the number of elements (max) for allocation */
            max = (pval > max) ? pval : max;
        }
    }
    /* extra to allow for varying taps dur to random tap */
    rvbdelsize = (int)(max * 1.2);
}

void
QRVB::set_random()
{
    static const float rnd_freq[QRVB_CHANS][QRVB_DELCOUNT] = {    /* the freq of rand variations */
            {4.9, 5.2, 6.7},
            {3.4, 2.7, 3.8},
            {5.1, 6.3, 7.0},
            {3.6, 2.8, 4.4}
    };
    static const float rnd_pcnt[QRVB_CHANS][QRVB_DELCOUNT] = {    /* the amt. of delay to vary */
            {.0016, .0013, .0021},
            {.0018, .0019, .0014},
            {.0027, .0014, .0020},
            {.0016, .0012, .0015}
    };
    static const float seed[QRVB_CHANS][QRVB_DELCOUNT] = {        /* the seeds for randi */
            {.314, .159, .265},
            {.358, .979, .323},
            {.142, .857, .685},
            {.246, .776, .456}
    };

    for (int i = 0; i < QRVB_CHANS; i++) {              /* loop for OutChannels chans worth */
        for (int j = 0; j < QRVB_DELCOUNT; j++) {           /* loop for DelayCount delays per chan */
            ReverbData *r = &m_rvbData[i][j];
            r->Rand_info[0] = rnd_pcnt[i][j] * SR / 2.0;    /* nsamps jitter */
            r->Rand_info[1] = cycle(SR, rnd_freq[i][j], 512);   /* SI  */
            r->Rand_info[2] = 1.0;
            r->Rand_info[4] = seed[i][j];     /* load seeds */
        }
    }
}

