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

extern "C" int get_path_params(float *rhos, float *thetas, int *cartesian, double *mdiff);

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


/* ------------------------------------------------------------ rtprofile --- */
void rtprofile()
{
   RT_INTRO("MOVE", makeMOVE);
}

// Move methods

MOVE::MOVE()
{
    R_old = -100000.0;
    T_old = 0.0;
    setup_trigfuns();
    rholoc = new float[ARRAYSIZE];
    thetaloc = new float[ARRAYSIZE];
    for (int n = 0; n < 2; n++)
        for (int o = 0; o < 13; o++)
	    oldOutlocs[n][o] = LocationUnset;	// to force update
}

MOVE::~MOVE()
{
    delete [] thetaloc;
    delete [] rholoc;
}

int MOVE::localInit(float *p, short n_args)
{
    if (n_args < 6)
        die(name(), "Wrong number of args.");
    m_dist = p[4];
    m_rvbamp = p[5];
    m_inchan = n_args > 6 ? (int)p[6] : AVERAGE_CHANS;
    
    // copy global params into instrument
    
    if (get_path_params(&rholoc[0], &thetaloc[0], &cartflag, &mindiff) < 0)
        return -1;

	// treat mindiff as update rate in seconds
	if (mindiff > 0.0)
	    setBufferSize((int) SR * mindiff);

    // tables for positional lookup
    
    tableset(m_dur, ARRAYSIZE, tabr);
    tableset(m_dur, ARRAYSIZE, tabt);
    
    return 0;
}

int MOVE::finishInit(double rvb_time, double *ringdur)
{
    *ringdur = rvb_time;	// default
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

// This gets called every internal buffer's worth of samples.
// The angle filters are only reset once every N degrees, where N is variable.

int MOVE::updatePosition(int currentSamp)
{
    double R = tablei(currentSamp, rholoc, tabr);
    double T = tablei(currentSamp, thetaloc, tabt);
    int maxtap = tapcount;
    int resetFlag = 0;	// no reset during update
    if (R != R_old || T != T_old) {
#ifdef debug
        printf("updatePosition: R: %f  T: %f\n", R, T);
#endif
	if (roomtrig(R , T, m_dist, cartflag)) {
            return (-1);
	}
        // set taps, return max samp
        maxtap = tap_set(m_binaural);
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
