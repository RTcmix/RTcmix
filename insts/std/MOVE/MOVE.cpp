// MOVE.C -- implementation of MOVE class

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ugens.h>
#include <mixerr.h>
#include <rt.h>
#include <rtdefs.h>

#include "MOVE.h"
#include "common.h"

#undef debug

#ifdef SIG_DEBUG
#define DBG(stmt) { stmt; }
#else
#define DBG(stmt)
#endif

extern "C" int get_path_params(double *rhos, double *thetas, int *cartesian, double *mdiff);

extern double SINARRAY[1025], COSARRAY[1025], ATANARRAY[1025];

static const double radpt = 162.99746617261;   /* converts rads to 1024-element array ptr */
static const double radpt2 = 325.9493234522;

inline double SIN(double x) { return SINARRAY[(int)(wrap(x) * radpt + 0.5)]; }
inline double COS(double x) { return COSARRAY[(int)(wrap(x) * radpt + 0.5)]; }
inline double ATAN(double x) { return ATANARRAY[(int)((x) * radpt2) + 512]; }

static const double LocationUnset = -999999.999999;

/* ------------------------------------------------------------ makeMOVE --- */
Instrument *makeMOVE()
{
   MOVE *inst;

   inst = new MOVE();
   inst->set_bus_config("MOVE");

   return inst;
}


#ifndef MAXMSP
/* ------------------------------------------------------------ rtprofile --- */
void rtprofile()
{
#ifdef USE_BUGGY_CODE
   RT_INTRO("OLDMOVE", makeMOVE);
#else
   RT_INTRO("MOVE", makeMOVE);
#endif
}
#endif

// Move methods

MOVE::MOVE()
{
    R_old = -100000.0;
    T_old = 0.0;
    m_updateCount = 0;
    m_updateSamps = BUFLEN;
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

int MOVE::localInit(double *p, int n_args)
{
    if (n_args < 6) {
        die(name(), "Wrong number of args.");
		  return(DONT_SCHEDULE);
	 }
    m_dist = p[4];
    m_rvbamp = p[5];
    m_inchan = n_args > 6 ? (int)p[6] : AVERAGE_CHANS;
    
    // copy global params into instrument
    
    if (get_path_params(&rholoc[0], &thetaloc[0], &cartflag, &mindiff) < 0) {
		  die(name(), "get_path_params failed.");
        return(DONT_SCHEDULE);
	}

	// treat mindiff as update rate in seconds
	if (mindiff > 0.0) {
		m_updateSamps = (int) (SR * mindiff + 0.5);
		if (m_updateSamps <= BUFLEN)
		{
	        setBufferSize(m_updateSamps);
#ifdef debug
			printf("buffer size reset to %d samples\n", getBufferSize());
#endif
		}
		// if update rate is larger than BUFLEN samples, set buffer size
		// to be some integer fraction of the desired update count, then
		// reset update count to be multiple of this.
		else {
		    int divisor = 2;
			int newBufferLen;
			while ((newBufferLen = m_updateSamps / divisor) > BUFLEN)
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
   register double otap = (double) tap - outloc;
   if (otap < 0.0) otap += m_tapsize;
   double otapPlusOne = otap + 1.0;
   if (otapPlusOne >= (double) m_tapsize) otapPlusOne -= m_tapsize;
   
   double closestToEnd = max(otap, otapPlusOne);

   // run till one of output taps wrap, or len reached

   int len1 = min(len, m_tapsize - (int) closestToEnd);

   register double *tapdel = m_tapDelay;
   register double *Sig = vec->Sig;
   int out = 0;
   
   while (out < len)
   {
       const int clen = len1;
       for (int i = 0; i < clen; ++i) {
	   int outtap = (int) otap;
	   double frac = otap - (double)outtap;
	   Sig[out++] = tapdel[outtap] + frac * (tapdel[outtap + 1] - tapdel[outtap]);
	   otap += incr;
       }

       if (otap >= (double) m_tapsize) otap -= m_tapsize;
       otapPlusOne = otap + 1.0;
       if (otapPlusOne >= (double) m_tapsize) otapPlusOne -= m_tapsize;
       closestToEnd = max(otap, otapPlusOne);

       len1 = min(len - out, m_tapsize - (int) closestToEnd);
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
		if (roomtrig(R , T, m_dist, cartflag)) {
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
