// DMOVE.C -- implementation of DMOVE class

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ugens.h>
#include <rt.h>
#include <rtdefs.h>

#include <common.h>
#include "DMOVE.h"

//#define debug

#ifdef SIG_DEBUG
#define DBG(stmt) { stmt; }
#else
#define DBG(stmt)
#endif

extern "C" int get_params(int *cartesian, double *mdiff);


/* ------------------------------------------------------------ makeMOVE --- */
Instrument *makeDMOVE()
{
   DMOVE *inst;

   inst = new DMOVE();
   inst->set_bus_config("DMOVE");

   return inst;
}

#ifndef EMBEDDED
extern Instrument *makeRVB();	// From RVB.C

/* ------------------------------------------------------------ rtprofile --- */
void rtprofile()
{
   RT_INTRO("DMOVE", makeDMOVE);
   RT_INTRO("RVB", makeRVB);
}
#endif

// Move methods

DMOVE::DMOVE() : MOVEBASE(RTBUFSAMPS)
{
}

DMOVE::~DMOVE()
{
}

int DMOVE::localInit(double *p, int n_args)
{
    if (n_args < 7)
        return die(name(), "Wrong number of args.");
    m_dist = p[6];
    if (m_dist < 0.0) {
        rtcmix_advise(name(), "Using cartesian coordinate system");
        m_dist *= -1.0;
        m_cartflag = 1;
    }
    m_inchan = n_args > 7 ? (int)p[7] : AVERAGE_CHANS;

    if (alloc_vectors() == DONT_SCHEDULE) {
        return DONT_SCHEDULE;
    }

    // copy global params into instrument
    int ignoreme;
    if (get_params(&ignoreme, &mindiff) < 0)
        return die(name(), "get_params failed.");

    // treat mindiff as update rate in seconds
    if (mindiff > 0.0) {
        m_updateSamps = (int) (SR * mindiff + 0.5);
        if (m_updateSamps <= RTBUFSAMPS)
        {
            setBufferSize(m_updateSamps);
#ifdef debug
            printf("buffer size reset to %d samples\n", getBufferSize());
#endif
        }
            // if update rate is larger than RTBUFSAMPS samples, set buffer size
            // to be some integer fraction of the desired update count, then
            // reset update count to be multiple of this.
        else {
            int divisor = 2;
            int newBufferLen;
            while ((newBufferLen = m_updateSamps / divisor) > RTBUFSAMPS)
                divisor++;
            setBufferSize(newBufferLen);
            m_updateSamps = newBufferLen * divisor;
#ifdef debug
            printf("buffer size reset to %d samples\n", getBufferSize());
#endif
        }
#ifdef debug
        printf("updating every %d samps\n", m_updateSamps);
#endif
    }

    return 0;
}

int DMOVE::finishInit(double *ringdur)
{
#ifdef debug
    printf("finishInit()\n");
#endif
    *ringdur = (float)m_tapsize / SR;	// max possible room delay
    tapcount = updatePosition(0);
    R_old = T_old = -99999999.0;	// force updatePosition() to do its work next time
    return 0;
}

// This gets called every internal buffer's worth of samples.  The actual
// update of source angles and delays only happens if we are due for an update
// (based on the update count) and there has been a change of position

int DMOVE::updatePosition(int currentSamp)
{
    double p[6];
    update(p, 6, 1 << 4 | 1 << 5);
    double R = p[4];
    double T = p[5];
    int maxtap = tapcount;
    if (R != R_old || T != T_old) {
#ifdef debug
        printf("updatePosition[%d]:\t\tR: %f  T: %f\n", currentSamp, R, T);
#endif
        double noOffset[] = {0};
        if (roomtrig(R , T, m_dist, noOffset, m_cartflag)) {
            return (-1);
        }
        // set taps, return max samp
        maxtap = tap_set(binaural());
        int resetFlag = (currentSamp == 0);
        airfil_set(resetFlag);
        if (binaural())
            earfil_set(resetFlag);
        else
            mike_set();
        R_old = R;
        T_old = T;
    }
    return maxtap;	// return new maximum delay in samples
}

void DMOVE::get_tap(int currentSamp, int chan, int path, int len) {
    SVector vec = m_vectors[chan][path];
    getVecTap(m_tapDelay, m_tapsize, vec->Sig, vec->outloc, &oldOutlocs[chan][path], currentSamp, len);
}