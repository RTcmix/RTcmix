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

static const double LocationUnset = -999999.999999;

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

DMOVE::DMOVE()
{
    R_old = -100000.0;
    T_old = -100000.0;
    m_updateSamps = RTBUFSAMPS;
    for (int n = 0; n < 2; n++)
        for (int o = 0; o < 13; o++)
	    oldOutlocs[n][o] = LocationUnset;	// to force update
}

DMOVE::~DMOVE()
{
}

// PFields:
// inskip, outskip, dur, amp, xpos, ypos, mike_dist, inchan

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

void DMOVE::get_tap(int currentSamp, int chan, int path, int len)
{
   Vector *vec = &m_vectors[chan][path];
   double outloc = vec->outloc;
   const double oldOutloc = oldOutlocs[chan][path];
   double delta;
   if (oldOutloc == LocationUnset)
       delta = 0.0;
   else
       delta = outloc - oldOutloc;
   oldOutlocs[chan][path] = outloc;
   
   double incr = 1.0 + delta / len;

   const int tap = currentSamp % m_tapsize;
   register double outTap = (double) tap - outloc;
   if (outTap < 0.0) outTap += m_tapsize;
   double *tapdel = m_tapDelay;
   double *Sig = vec->Sig;
    int out = 0;
    
    int len1 = (m_tapsize - (outTap + 1.0)) / incr;     // boundary point before needing to wrap
    int count = 0;
    int sampsNeeded = len;
    while (sampsNeeded > 0 && count++ < len1) {
        const int iOuttap = (int) outTap;
        const double frac = outTap - (double)iOuttap;
        Sig[out++] = tapdel[iOuttap] + frac * (tapdel[iOuttap+1] - tapdel[iOuttap]);
        outTap += incr;
        --sampsNeeded;
    }
    while (sampsNeeded > 0 && outTap < (double) m_tapsize) {
        const int iOuttap = (int) outTap;
        const double frac = outTap - (double)iOuttap;
        int outTapPlusOne = int(outTap + 1.0);
        if (outTapPlusOne >= m_tapsize)
            outTapPlusOne -= m_tapsize;
        Sig[out++] = tapdel[iOuttap] + frac * (tapdel[outTapPlusOne] - tapdel[iOuttap]);
        outTap += incr;
        --sampsNeeded;
    }
    outTap -= m_tapsize;
    while (sampsNeeded > 0) {
        const int iOuttap = (int) outTap;
        const double frac = outTap - (double)iOuttap;
        Sig[out++] = tapdel[iOuttap] + frac * (tapdel[iOuttap+1] - tapdel[iOuttap]);
        outTap += incr;
        --sampsNeeded;
    }
}

// This gets called every internal buffer's worth of samples.  The actual
// update of source angles and delays only happens if we are due for an update
// (based on the update count) and there has been a change of position

int DMOVE::updatePosition(int currentSamp)
{
    static double z = 0.017453292;    /* Pi/180 */
    double p[6];
	update(p, 6, 1 << 4 | 1 << 5);
    double R = p[4];
    double T = p[5] * z;    // convert to radians
    int maxtap = tapcount;
    if (R != R_old || T != T_old) {
#ifdef debug
        printf("updatePosition[%d]:\t\tR: %f  T: %f\n", currentSamp, R, T);
#endif
		if (roomtrig(R , T, m_dist, m_cartflag)) {
            return (-1);
		}
        // set taps, return max samp
        maxtap = tap_set(m_binaural);
 	    int resetFlag = (currentSamp == 0);
		airfil_set(resetFlag);
		if (m_binaural)
		   earfil_set(resetFlag);
		else
		   mike_set();
		R_old = R;
		T_old = T;
    }
    return maxtap;	// return new maximum delay in samples
}
