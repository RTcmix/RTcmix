#include "MPLACE.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ugens.h>
#include <mixerr.h>
#include <rt.h>
#include <rtdefs.h>

#include <common.h>
#include "msetup.h"

//#define debug
//#define SIG_DEBUG
//#define DELAY_DEBUG

#ifdef SIG_DEBUG
#define DBG(stmt) { stmt; }
#else
#define DBG(stmt)
#endif


extern int g_Nterms[13];                 /* defined in ../MOVE/common.C */

/* ---------------------------------------------------------------- PLACE --- */
PLACE::PLACE()
{
}


/* --------------------------------------------------------------- ~PLACE --- */
PLACE::~PLACE()
{
}

int PLACE::localInit(double p[], int n_args)
{
    if (n_args < 7)
        return die(name(), "Wrong number of args.");
	
	const double conv = PI2 / 360.0;
	double R = (double)p[4];
	double T = (double)p[5];
	m_dist = (double)p[6];
	m_inchan = n_args > 7 ? (int)p[7] : AVERAGE_CHANS;
    if (m_dist < 0) {
        m_cartflag = 1;                    /* cartesian coordinates */
        m_dist *= -1.0;
        rtcmix_advise(name(), "Using cartesian coordinate system.");
    }
	// convert angle to radians before passing in if polar
    if (roomtrig(R , m_cartflag ? T : T * conv, m_dist, m_cartflag)) {
		  die(name(), "roomtrig failed.");
        return(DONT_SCHEDULE);
    }
    return 0;
}

int PLACE::finishInit(double *ringdur)
{
   /* set taps, return max samp */
   tapcount = tap_set(m_binaural);

   // Set up air and other filters.
   int resetFlag = 1;
   airfil_set(resetFlag);

   *ringdur = (float)tapcount / SR;

   return 0;
}

int PLACE::configure()
{
	int status = MBASE::configure();
	if (status == 0) {
		// PLACE sets all filters just once, so we clear them at this time
		int flag = 1;
		airfil_set(flag);
		if (m_binaural)
		   earfil_set(flag);
		else
		   mike_set();
	}
	return status;
}

int PLACE::updatePosition(int)
{
    return tapcount;	// no-op for this class
}

/* ------------------------------------------------------------ makePLACE --- */
Instrument *makeMPLACE()
{
   PLACE *inst;

   inst = new PLACE();
   inst->set_bus_config("MPLACE");

   return inst;
}

extern Instrument *makeRVB();	// From RVB.C

/* ------------------------------------------------------------ rtprofile --- */
void rtprofile()
{
   RT_INTRO("MPLACE", makeMPLACE);
   RT_INTRO("RVB", makeRVB);
}

/* -------------------------------------------------------------- get_tap --- */
/* Accesses the tap delay array and fills the del. signal array, Sig. */

void PLACE::get_tap(int intap, int chan, int path, int len)
{
	Vector *vec = &m_vectors[chan][path];
	register double *tapdel = m_tapDelay;
	register double *Sig = vec->Sig;
	register int tap = intap % m_tapsize;
    int outtap = tap - (int) vec->outloc;
    if (outtap < 0) outtap += m_tapsize;

	int len1 = min(len, m_tapsize - outtap);
	int i;

    // run till the output tap wraps, or len reached

	for (i = 0; i < len1; ++i) {
    	Sig[i] = tapdel[outtap++];
	}
	
	// wrap outtap and finish if necessary
	
	if (outtap >= m_tapsize)
	    outtap -= m_tapsize;

	for (; i < len; ++i) {
    	Sig[i] = tapdel[outtap++];
	}
}

