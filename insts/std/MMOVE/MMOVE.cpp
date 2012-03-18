// MMOVE.C -- implementation of MMOVE class

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ugens.h>
#include <mixerr.h>
#include <rt.h>
#include <rtdefs.h>
#include <assert.h>

#include <common.h>
#include "MMOVE.h"

//#define debug

#ifdef SIG_DEBUG
#define DBG(stmt) { stmt; }
#else
#define DBG(stmt)
#endif

extern "C" int get_path_params(double *rhos, double *thetas, int *cartesian, double *mdiff);

// Defined in ../MOVE/common.C
extern double SINARRAY[1025], COSARRAY[1025], ATANARRAY[1025];

static const double radpt = 162.99746617261;   /* converts rads to 1024-element array ptr */
static const double radpt2 = 325.9493234522;

inline double SIN(double x) { return SINARRAY[(int)(wrap(x) * radpt + 0.5)]; }
inline double COS(double x) { return COSARRAY[(int)(wrap(x) * radpt + 0.5)]; }
inline double ATAN(double x) { return ATANARRAY[(int)((x) * radpt2) + 512]; }

static const double LocationUnset = -999999.999999;

/* ------------------------------------------------------------ makeMOVE --- */
Instrument *makeMMOVE()
{
   MOVE *inst;

   inst = new MOVE();	// The class is called MOVE, but the inst is MMOVE
   inst->set_bus_config("MMOVE");

   return inst;
}

extern Instrument *makeRVB();	// From RVB.C

/* ------------------------------------------------------------ rtprofile --- */
void rtprofile()
{
   RT_INTRO("MMOVE", makeMMOVE);
   RT_INTRO("RVB", makeRVB);
}

// Move methods

MOVE::MOVE()
{
    R_old = -100000.0;
    T_old = 0.0;
    m_updateCount = 0;
    m_updateSamps = RTBUFSAMPS;
    setup_trigfuns();
    rholoc = new double[ARRAYSIZE];
    thetaloc = new double[ARRAYSIZE];
    for (int n = 0; n < 2; n++)
        for (int o = 0; o < 13; o++)
	    oldOutlocs[n][o] = LocationUnset;	// to force update
}

MOVE::~MOVE()
{
    delete [] thetaloc;
    delete [] rholoc;
}

#define MINBUFFERSIZE 64

int MOVE::localInit(double *p, int n_args)
{
    if (n_args < 6)
        return die(name(), "Wrong number of args.");
    m_dist = p[4];
    m_inchan = n_args > 5 ? (int)p[5] : AVERAGE_CHANS;
    
    // copy global params into instrument
    
    if (get_path_params(&rholoc[0], &thetaloc[0], &m_cartflag, &mindiff) < 0)
		  return die(name(), "get_path_params failed.");

	// treat mindiff as update rate in seconds
	if (mindiff > 0.0) {
		m_updateSamps = (int) (SR * mindiff + 0.5);
		if (m_updateSamps <= RTBUFSAMPS) {
			if (m_updateSamps < MINBUFFERSIZE)
				m_updateSamps = MINBUFFERSIZE;
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

    // tables for positional lookup
    
    tableset(SR, m_dur, ARRAYSIZE, tabr);
    tableset(SR, m_dur, ARRAYSIZE, tabt);
    
    return 0;
}

int MOVE::finishInit(double rvb_time, double *ringdur)
{
    *ringdur = rvb_time;	// default
	tapcount = updatePosition(0);
    return 0;
}

void MOVE::get_tap(int currentSamp, int chan, int path, int len)
{
   Vector *vec = &m_vectors[chan][path];
   const double outloc = (double) vec->outloc;
   const double oldOutloc = oldOutlocs[chan][path];
   const double delta = (oldOutloc == LocationUnset) ? 0.0 : outloc - oldOutloc;
   
   oldOutlocs[chan][path] = outloc;
   
   const double incr = 1.0 + delta / len;
   const int tap = currentSamp % m_tapsize;
   
   register double *tapdel = m_tapDelay;
   register double *Sig = vec->Sig;
   register double outTap = (double) tap - outloc;
   if (outTap < 0.0)
	   outTap += m_tapsize;
   int out = 0;
   
   while (out < len)
   {
		if (outTap >= (double) m_tapsize)
			outTap -= m_tapsize;   
		int iouttap = (int) outTap;
		int outTapPlusOne = int(outTap + 1.0);
		if (outTapPlusOne >= m_tapsize)
			outTapPlusOne -= m_tapsize;   
		double frac = outTap - (double)iouttap;
		Sig[out++] = tapdel[iouttap] + frac * (tapdel[outTapPlusOne] - tapdel[iouttap]);
		outTap += incr;
   }
}

// This gets called every internal buffer's worth of samples.  The actual
// update of source angles and delays only happens if we are due for an update
// (based on the update count) and there has been a change of position

int MOVE::updatePosition(int currentSamp)
{
    double R = tablei(currentSamp, rholoc, tabr);
    double T = tablei(currentSamp, thetaloc, tabt);
    int maxtap = tapcount;
	m_updateCount -= getBufferSize();
    if (m_updateCount <= 0 && (R != R_old || T != T_old)) {
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
		m_updateCount = m_updateSamps;	// reset counter
    }
    return maxtap;	// return new maximum delay in samples
}
