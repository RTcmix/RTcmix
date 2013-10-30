// MBASE.C -- common base class implementation for MMOVE and MPLACE

#include "MBASE.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ugens.h>
#include <mixerr.h>
#include <assert.h>
#include <rt.h>
#include <rtdefs.h>

#include <PField.h>

#include <common.h>
#include "msetup.h"

//#define debug
//#define SIG_DEBUG
// #define LOOP_DEBUG
// #define DELAY_DEBUG
//#define MYDEBUG

#ifdef SIG_DEBUG
#define DBG(stmt) { stmt; }
#else
#define DBG(stmt)
#endif

#if defined (MYDEBUG) || defined(SIG_DEBUG) || defined(LOOP_DEBUG)
#define DBG1(stmt) { stmt; }
#else
#define DBG1(stmt)
#endif

#define SIG_THRESH 100000000.0

extern "C" {
   #include "cmixfuns.h"
}

extern const int g_Nterms[13];                 /* defined in ../MOVE/common.C */

MBASE::MBASE() : m_tapsize(0)
{
    in = NULL;
    m_tapDelay = NULL;
	m_buffersize = RTBUFSAMPS;
	m_paths = 13;
	m_cartflag = 0;

    for (int i = 0; i < 2; i++) {
       int j;
       for (j = 0; j < m_paths; j++) {
		  m_vectors[i][j].Sig = NULL;
          m_vectors[i][j].Firtaps = NULL;
          m_vectors[i][j].Fircoeffs = NULL;
       }
    }
	increment_users();
}

MBASE::~MBASE()
{
   delete [] in;
   delete [] m_tapDelay;
   int i, j;
   for (i = 0; i < 2; i++) {
      for (j = 0; j < 13; j++) {
		 delete [] m_vectors[i][j].Sig;
         delete [] m_vectors[i][j].Firtaps;
         delete [] m_vectors[i][j].Fircoeffs;
      }
   }
	decrement_users();
}

int MBASE::init(double p[], int n_args)
{
    int    flag, UseMikes;
    float  outskip, inskip, abs_factor, dummy;
    double R, T, dist;

    outskip = p[0];
    inskip = p[1];
    m_dur = p[2];
    if (m_dur < 0)                      /* "dur" represents timend */
        m_dur = -m_dur - inskip;

    if (rtsetinput(inskip, this) == -1) { // no input
		  return(DONT_SCHEDULE);
	}
    insamps = (int)(m_dur * SR);
	
   	inamp = p[3];

	if (inamp < 0) {
		m_paths = 1;	// Dont process secondary paths
		inamp = -inamp;
	}

    /* Get results of Minc setup calls (space, mikes_on, mikes_off, matrix) */
    if (get_setup_params(Dimensions, &m_attenParams,
						 &dummy, &abs_factor, &UseMikes, &MikeAngle,
						 &MikePatternFactor) == -1) {
		return die(name(), "You must call setup routine `space' first.");
	}

	// call inst-specific init code
    if (localInit(p, n_args) == DONT_SCHEDULE)
		  return die(name(), "localInit failed.");

    if (m_inchan >= inputChannels())
       return die(name(), "You asked for channel %d of a %d-channel input file.",
                  m_inchan, inputChannels());

    if (inputChannels() == 1)
       m_inchan = 0;
	
	if (outputChannels() != 4)
		return die(name(), "Output must be 4-channel (2 signal, 2 reverb feed).");
	
    /* (perform some initialization that used to be in space.c) */
    int meanLength = MFP_samps(SR, Dimensions); // mean delay length for reverb
    get_lengths(meanLength);              /* sets up delay lengths */
    set_gains();                		/* sets gains for filters */
    set_walls(abs_factor);              /* sets wall filts for move routine */


   /* flag for use of ear filters */
   m_binaural = (!UseMikes && m_dist < 0.8 && m_dist != 0.0);

   amparray = floc(1);
   if (amparray) {
      int amplen = fsize(1);
      tableset(SR, m_dur, amplen, amptabs);      /* controls input dur only */
   }
   
   /* determine extra run time for this routine before calling rtsetoutput() */
   double reflectionDur = 0.0;
   finishInit(&reflectionDur);
   
   m_branch = 0;
   
   if (rtsetoutput(outskip, m_dur + reflectionDur, this) == -1)
      return DONT_SCHEDULE;
   DBG1(printf("nsamps = %d\n", nSamps()));
   return nSamps();
}

static inline void PrintInput(float *sig, int len, double threshold = 0.0)
{
    for (int i = 0; i < len; i++)
	    if (sig[i] > threshold || sig[i] < -threshold)
		    printf("sig[%d] = %f\n", i, sig[i]);
	printf("\n");
}

static inline void PrintOutput(float *sig, int len, int chans, double threshold = 0.0)
{
	const int samps = len * chans;
	bool doPrint = false;
    for (int i = 0; i < samps; i += chans) {
	    if (sig[i] > threshold || sig[i] < -threshold)
			doPrint = true;
		if (doPrint)
		    printf("sig[%d] = %f\n", i/chans, sig[i]);
	}
	printf("\n");
}

static inline void PrintSig(double *sig, int len, double threshold = 0.0)
{
	bool doPrint = false;
    for (int i = 0; i < len; i++) {
	    if (sig[i] > threshold || sig[i] < -threshold)
			doPrint = true;
		if (doPrint)
		    printf("sig[%d] = %f\n", i, sig[i]);
	}
	printf("\n");
}

int MBASE::getInput(int currentFrame, int frames)
{
    // number of samples to process this time through
	const int inChans = inputChannels();

    int rsamps = frames * inChans;

    rtgetin(in, this, rsamps);
	
    int n = 0;
    int lCurSamp;	// local copy for inner loops
    float insig;
#ifdef LOOP_DEBUG
    int nsig = 0, nzeros = 0;
#endif
	float scale = 1.0/inChans;
    // apply curve to input signal and mix down to mono if necessary

    for (int s = 0, curFrm = currentFrame; s < rsamps; s += inChans, curFrm++)
    {
		if (curFrm < insamps) {	/* processing input signal */
#ifdef LOOP_DEBUG
			nsig++;
#endif
			if (--m_branch < 0) {
			   inamp = update(3, insamps, curFrm);
			   if (amparray)
    			  inamp *= tablei(curFrm, amparray, amptabs);
			   m_branch = getSkip();
			}
			if (m_inchan == AVERAGE_CHANS) {
			   insig = 0.0;
			   for (int c = 0; c < inChans; c++)
    			  insig += in[s + c];
			   insig *= scale;
			}
			else
			   insig = in[s + m_inchan];
			insig *= inamp;
		}
		else  {                               /* flushing delays & reverb */
#ifdef LOOP_DEBUG
			nzeros++;
#endif
 			insig = 0.0;
		}
	    in[n++] = insig;	// write back into input array to save space
    }
#ifdef LOOP_DEBUG
	DBG1(printf("getInput(): %d signal, %d zero padded\n", nsig, nzeros));
#endif
	return 0;
}

int MBASE::configure()
{
	int status = 0;
	
	in = new float [RTBUFSAMPS * inputChannels()];
    status = alloc_delays();			/* allocates memory for delays */
	
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 13; j++)
			m_vectors[i][j].Sig = new double[getBufferSize()];

	rvb_reset(m_tapDelay);   			// resets tap delay

	if (status == 0 && m_binaural) {
		rtcmix_advise(name(), "Running in binaural mode.");
		status = alloc_firfilters();	// allocates memory for FIRs
	}
	return status;
}

/* ------------------------------------------------------------------ run --- */
int MBASE::run()
{
	const int totalSamps = insamps + tapcount;
	int thisFrame = currentFrame();
	const int outChans = outputChannels();
	
	DBG1(printf("%s::run(): totalSamps = %d\n", name(), totalSamps));

	// this will return chunksamps' worth of input, even if we have
	// passed the end of the input (will produce zeros)

	getInput(thisFrame, framesToRun());

	DBG1(printf("getInput(%d, %d) called\n", thisFrame, framesToRun()));
	
	int bufsamps = getBufferSize();
	const int outputOffset = this->output_offset;
	
	// loop for required number of output samples
	const int frameCount = framesToRun();
	
	memset(this->outbuf, 0, frameCount * outChans * sizeof(BUFTYPE));
	
	int frame = 0;
	while (frame < frameCount) {
		// limit buffer size to end of current pull (chunksamps)
		if (frameCount - frame < bufsamps)
            bufsamps = max(0, frameCount - frame);

		thisFrame = currentFrame();	// store this locally for efficiency

		DBG1(printf("top of main loop: frame = %d  thisFrame = %d  bufsamps = %d\n",
                   frame, thisFrame, bufsamps));
		DBG(printf("input signal:\n"));
		DBG(PrintInput(&in[frame], bufsamps));
		
		// add signal to delay
		put_tap(thisFrame, &in[frame], bufsamps);

		// if processing input signal or flushing delay lines ... 

		if (thisFrame < totalSamps) {
			// limit buffer size of end of input data
			if (totalSamps - thisFrame < bufsamps)
				bufsamps = max(0, totalSamps - thisFrame);

			if ((tapcount = updatePosition(thisFrame)) < 0)
				exit(-1);

			DBG1(printf("  vector loop: bufsamps = %d\n", bufsamps));
			for (int ch = 0; ch < 2; ch++) {
				for (int path = 0; path < m_paths; path++) {
					Vector *vec = &m_vectors[ch][path];
					/* get delayed samps */
					get_tap(thisFrame, ch, path, bufsamps);
#if 0				
                    DBG(printf("signal [%d][%d] before filters:\n", ch, path));
					DBG(PrintSig(vec->Sig, bufsamps));
#endif
					/* air absorpt. filters */
         			air(vec->Sig, bufsamps, vec->Airdata);			
					/* wall absorpt. filters */
					if (path > 0)	// no filtering of direct signal
         				wall(vec->Sig, bufsamps, vec->Walldata);
					/* do binaural angle filters if necessary*/
					if (m_binaural)	{					
						fir(vec->Sig, thisFrame, g_Nterms[path], 
					    	vec->Fircoeffs, vec->Firtaps, bufsamps);
					}
                    DBG(printf("signal [%d][%d] before rvb:\n", ch, path));
                    DBG(PrintSig(vec->Sig, bufsamps));
//                    DBG(PrintSig(vec->Sig, bufsamps, SIG_THRESH));
		 		}
			}
			DBG(printf("summing vectors\n"));
			Vector *vec;
			register float *outptr = &this->outbuf[frame*outChans];
			// sum unscaled reflected paths as global input for RVB.
			for (int path = 0; path < m_paths; path++) {
				vec = &m_vectors[0][path];
				addScaleBufToOut(&outptr[2], vec->Sig, bufsamps, outChans, 1.0);
				vec = &m_vectors[1][path];
				addScaleBufToOut(&outptr[3], vec->Sig, bufsamps, outChans, 1.0);
			}
			if (!m_binaural) {
				// now do cardioid mike effect 
				// add scaled reflected paths to output as early response
				for (int path = 1; path < m_paths; path++) {
					vec = &m_vectors[0][path];
					addScaleBufToOut(&outptr[0], vec->Sig, bufsamps, outChans, vec->MikeAmp);
					vec = &m_vectors[1][path];
					addScaleBufToOut(&outptr[1], vec->Sig, bufsamps, outChans, vec->MikeAmp);
#if 0
					DBG(printf("early response L and R:\n"));
					DBG(PrintOutput(&outptr[0], bufsamps, outChans, SIG_THRESH));
					DBG(PrintOutput(&outptr[1], bufsamps, outChans, SIG_THRESH));
#endif
				}           
			}
			else {
           		// copy scaled, filtered reflected paths (reverb input) as the early reponse
				// to the output
				for (int ch = 0; ch < 2; ++ch) {
					float *dest = &outptr[ch];
					float *src = &outptr[ch+2];
					for (int n=0; n<bufsamps; ++n) {
						*dest = *src;
						dest += outChans;
						src += outChans;
					}
				}
			}
			/* add the direct signal into the output bus  */
			for (int n = 0; n < bufsamps; n++) {
				outptr[0] += m_vectors[0][0].Sig[n];
				outptr[1] += m_vectors[1][0].Sig[n];
				outptr += outChans;
			}
			DBG(printf("FINAL MIX LEFT CHAN:\n"));
			DBG(PrintOutput(&this->outbuf[frame*outChans], bufsamps, outChans));
		}		
		increment(bufsamps);
		frame += bufsamps;
		bufsamps = getBufferSize();		// update
		DBG1(printf("\tmain loop done.  thisFrame now %d\n", currentFrame()));
	}
	DBG1(printf("%s::run done\n\n", name()));
	return frame;
}


/* -------------------------------------------------------------------------- */
/* The following functions from the original space.c. */
/* -------------------------------------------------------------------------- */

int MBASE::alloc_firfilters()
{
   /* allocate memory for FIR filters and zero delays */
   for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 13; j++) {
         m_vectors[i][j].Firtaps = new double[g_Nterms[j] + 1];
		 if (m_vectors[i][j].Firtaps == NULL) {
		 	rterror("MBASE (alloc_firfilters/Firtaps)", "Memory failure during setup");
			return -1;
		 }
		 memset(m_vectors[i][j].Firtaps, 0, (g_Nterms[j] + 1) * sizeof(double));
         m_vectors[i][j].Fircoeffs = new double[g_Nterms[j]];
		 if (m_vectors[i][j].Fircoeffs == NULL) {
		 	rterror("MBASE (alloc_firfilters/Fircoeffs)", "Memory failure during setup");
			return -1;
		 }
      }
   }
   return 0;
}

/* --------------------------------------------------------- alloc_delays --- */
/* Sets aside the memory needed for tap delay
*/
int MBASE::alloc_delays()
{
	assert(m_tapsize > 0);
    m_tapDelay = new double[m_tapsize + 8];
	if (m_tapDelay == NULL) {
		rterror("MBASE (alloc_delays)", "Memory failure during setup");
		return -1;
	}
	memset(m_tapDelay, 0, (m_tapsize + 8) * sizeof(double));
	return 0;
}


/* ---------------------------------------------------------- get_lengths --- */
/* Computes the length for the tap delay line (m_tapsize).
*/
void MBASE::get_lengths(long m_length)
{
   double diag, maxdim, mindim, d1, d0;

   /* get the length for the main tap delay */

   d0 = Dimensions[0] - Dimensions[2];
   d1 = Dimensions[1] - Dimensions[3];
   maxdim = (d0 > d1) ? d0 : d1;
   mindim = (d0 > d1) ? d1 : d0;
   diag = hypot((3 * maxdim), mindim);
   m_tapsize = (int)(diag / MACH1 * SR + 32);
#ifdef DELAY_DEBUG
   printf("tap delay has length %d\n", (m_tapsize + 8));
#endif
}


/* ------------------------------------------------------------ set_gains --- */
/* Creates the global array of AIR coefficients.
*/
void MBASE::set_gains()
{
   int    i, nvals = 16;
   static const float array[16] = {
      0, .001, 10, .1, 25, .225, 35, .28, 50, .35, 65, .4, 85, .45, 95, .475
   };

   /* compensate for SR differences */
   double adjust = 1.0 - (0.42 * (SR - 25000) / 25000.0);

   /* create scaled curve for coeffs */
   setline((float *)array, nvals, NCOEFFS, AIRCOEFFS);

   for (i = 0; i < NCOEFFS; i++)
      AIRCOEFFS[i] = pow(AIRCOEFFS[i], adjust);
}


/* ------------------------------------------------------------ set_walls --- */
/* Initializes the tone filters for simulating wall absorption.
   <wallfac> is a value between 0 and 10.
*/
void MBASE::set_walls(float wallfac)
{
   float cutoff, cf;

   wallfac /= 10.0;
   wallfac *= wallfac;                 /* now an expon. value between 0 and 1 */
   cutoff = wallfac * SR / 2;          /* sets -3db pt. between 0 & N.F. */
   cutoff = (cutoff <= SR / 2) ? cutoff : SR / 2;     /* set limit at N.F. */

   for (int i = 0; i < 2; i++) {
      for (int j = 1; j < m_paths; j++) {      /* skip first pair (direct sigs) */
         cf = (j > 4) ? cutoff * .6 : cutoff;   /* more filt for 2nd */
         toneset(SR, cf, 1, m_vectors[i][j].Walldata);        /* gen. wall reflect */
      }
   }
}


/* -------------------------------------------------------------------------- */
/* The following functions from the original placelib.c. */
/* -------------------------------------------------------------------------- */


/* ------------------------------------------------------------- roomtrig --- */
/* roomtrig calculates all distance/angle vectors for images up through the
   2nd generation reflections. 13 vectors total:  1 source, 4 1st gen., 8
   2nd gen. These are split into stereo pairs by binaural.
*/
int MBASE::roomtrig(double A,                 /* 'rho' or 'x' */
                    double B,                 /* 'theta' or 'y' */
                    double H,
                    int    cart)
{
   register int i;
   double x[13], y[13], r[13], t[13], d[4], Ra[2], Ta[2];
   double X, Y, R, T;
   const double z = 0.017453292;  /* Pi / 180 */

   /* calc. X & Y if entered in polar form only */

   if (!cart) {                 /* polar coordinates */
      R = A;
      T = B;	/* already in radians */
      X = A * sin(T);
      Y = A * cos(T);
   }
   else {
      R = hypot(A, B);
      T = atan(A / B);
      if (B < 0.0)
         T += M_PI;
      X = A;
      Y = B;
   }

   /* Check to see that source loc. is inside room bounds */

   if (X < Dimensions[3] || X > Dimensions[1] || Y > Dimensions[0] ||
       Y < Dimensions[2]) {
	  char msg[120];
      if (cart) sprintf(msg, "Source location [%.1f, %.1f] is outside room bounds!!", X, Y);
      else sprintf(msg, "Source location [%.1f, %.1f] is outside room bounds!!", R, T);
      rterror(name(), msg);
      return (1);
   }

   /* multiply global dimension array by 2 to save calc. time */

   for (i = 0; i < 4; ++i)
      d[i] = Dimensions[i] * 2.0;

   /* image calculations */

   /* source vector */
   x[0] = X;
   y[0] = Y;
   t[0] = T;
   r[0] = R;

   /* front wall vector */
   x[1] = X;
   y[1] = d[0] - Y;
   t[1] = atan(x[1] / y[1]);
   r[1] = y[1] / cos(t[1]);

   /* right wall vector */
   x[2] = d[1] - X;
   y[2] = Y;
   t[2] = PI / 2.0 - atan(y[2] / x[2]);
   r[2] = x[2] / sin(t[2]);

   /* back wall vector */
   x[3] = X;
   y[3] = d[2] - Y;
   t[3] = PI + atan(x[3] / y[3]);
   r[3] = y[3] / cos(t[3]);

   /* left wall vector */
   x[4] = d[3] - X;
   y[4] = Y;
   t[4] = 3. * PI / 2. - atan(y[4] / x[4]);
   r[4] = x[4] / sin(t[4]);

   /* 2nd gen. images: 4 opposing wall, 4 adjacent wall reflections */

   /* front wall vector */
   x[5] = X;
   y[5] = d[0] - d[2] + Y;
   t[5] = atan(x[5] / y[5]);
   r[5] = hypot(X, y[5]);

   /* right wall vector */
   x[6] = d[1] - d[3] + X;
   y[6] = Y;
   t[6] = PI / 2.0 - atan(y[6] / x[6]);
   r[6] = hypot(x[6], Y);

   /* back wall vector */
   x[7] = X;
   y[7] = d[2] - d[0] + Y;
   t[7] = PI + atan(x[7] / y[7]);
   r[7] = hypot(X, y[7]);

   /* left wall vector */
   x[8] = d[3] - d[1] + X;
   y[8] = Y;
   t[8] = 3.0 * PI / 2.0 - atan(y[8] / x[8]);
   r[8] = hypot(x[8], Y);

   /* fr. rt. vector - double image in rectangular room, as are next 3 */
   x[9] = x[2];
   y[9] = y[1];
   t[9] = atan(x[9] / y[9]);
   r[9] = hypot(x[9], y[9]);

   /* back rt. vector */
   x[10] = x[2];
   y[10] = y[3];
   t[10] = PI / 2.0 - atan(y[10] / x[10]);
   r[10] = hypot(x[10], y[10]);

   /* back lft. vector */
   x[11] = x[4];
   y[11] = y[3];
   t[11] = PI + atan(x[11] / y[11]);
   r[11] = hypot(x[11], y[11]);

   /* front lft. vector */
   x[12] = x[4];
   y[12] = y[1];
   t[12] = 3.0 * PI / 2.0 - atan(y[12] / x[12]);
   r[12] = hypot(x[12], y[12]);

   /* calculate stereo vector pairs for each of these */
   for (i = 0; i < 13; ++i) {
      binaural(r[i], t[i], x[i], y[i], H, Ra, Ta);
	  register Vector *lvec = &m_vectors[0][i];
	  register Vector *rvec = &m_vectors[1][i];
      lvec->Rho = Ra[0];
      rvec->Rho = Ra[1];
      lvec->Theta = Ta[0];
      rvec->Theta = Ta[1];
   }

   /* Check to see that source distance is not "zero" unless we have a min distance */

   if (m_attenParams.minDistance == 0.0 && 
       (m_vectors[0][0].Rho < 0.001 || m_vectors[1][0].Rho < 0.001)) {
      rterror(name(), "Zero source distance not allowed!");
      return (1);
   }

   /* print out data for analysis */
#ifdef debug
   printf("Source angles: %.2f    %.2f\n", m_vectors[0][0].Theta / z, m_vectors[1][0].Theta / z);
   printf("All others\n");
   for (i = 1; i < 13; i++)
      printf("%.2f     %.2f\n", m_vectors[0][i].Theta / z, m_vectors[1][i].Theta / z);
   printf("Direct delays:\n");
   printf("%d: %.2f ms.       %.2f ms.\n", i, m_vectors[0][0].Rho / 1.08, m_vectors[1][0].Rho / 1.08);
   printf("Room delays:\n");
   for (i = 1; i < 13; i++)
      printf("%.2f ms.       %.2f ms.\n ", m_vectors[0][i].Rho / 1.08, m_vectors[1][i].Rho / 1.08);
   printf("\n");
   printf("Direct dists:\n");
   printf("%d: %.2f ft.       %.2f ft.\n", i, m_vectors[0][0].Rho, m_vectors[1][0].Rho);
   printf("Room dists:\n");
   for (i = 1; i < 13; i++)
      printf("%.2f ft.       %.2f ft.\n ", m_vectors[0][i].Rho, m_vectors[1][i].Rho);
   printf("\n");
#endif

   return 0;
}


/* ------------------------------------------------------------ rvb_reset --- */
/* reset zeroes out all delay and filter histories in move and reverb
   each time move is called
*/
void MBASE::rvb_reset(double *m_tapDelay)
{
	register int i, j, k;

	/* reset wall filter hists */

	for (i = 0; i < 2; ++i) {
		for (j = 0; j < 13; ++j)
			m_vectors[i][j].Walldata[2] = 0.0;
	}

	/* reset tap delay */

	for (i = 0; i < m_tapsize + 8; ++i)
		m_tapDelay[i] = 0.0;
}


/* --------------------------------------------------------------- setair --- */
/* setair calculates and loads the gain factors (G1 & G2) for the tone
   filters used to simulate air absorption. The values for G1 are stored
   in AIRCOEFFS by space(). G2 takes the 1/r**n attenuation factor into
   account.  Histories are reset to 0 if flag = 1.
*/
void MBASE::setair(double rho, int flag, double *coeffs, bool directSrc)
{
   // Max rho distance for filter is 300 ft.

   double filt_rho = (rho > 300 ? 300 : rho);
   float fpoint = filt_rho * 511 / 300.0;
   int gpoint = (int)fpoint;
   float frac = fpoint - (float)gpoint;
   double G1 = AIRCOEFFS[gpoint] + frac * (AIRCOEFFS[gpoint + 1] - AIRCOEFFS[gpoint]);
   
   // Limit *direct* src gain attenuation by minDistance and maxDistance
   double rhoLimit = rho;
   if (directSrc) {
      rhoLimit = (rho < m_attenParams.minDistance) ? 
   						m_attenParams.minDistance :
							(rho > m_attenParams.maxDistance) ?
								m_attenParams.maxDistance : rho;
   }
   // atten = (dist/min_distance) ** -exponent
   double atten = pow(rhoLimit/m_attenParams.minDistance,
   					  -m_attenParams.distanceExponent);
//   if (directSrc)
//	   printf("rho was %g, min is %g, max is %g, rhoLimit is %g, attenuation = %g, expon = %g\n",
//			   rho, m_attenParams.minDistance, m_attenParams.maxDistance, rhoLimit, atten, -m_attenParams.distanceExponent);
   double G2 = atten * (1.0 - G1);

   /* load into output array */

   coeffs[0] = G2;
   coeffs[1] = G1;

   /* reset if flag = 1 */

   if (flag)
      coeffs[2] = 0.0;
}


/* ----------------------------------------------------------- airfil_set --- */
/* airfil_set is called by move to set all airfilters (2 for each image
   pair) whenever source moves.
*/
void MBASE::airfil_set(int flag)
{
   for (int i = 0; i < 2; ++i)
      for (int j = 0; j < m_paths; ++j)
         setair(m_vectors[i][j].Rho, flag, m_vectors[i][j].Airdata, j==0);
}

/* -------------------------------------------------------------- put_tap --- */
/* Accesses the tap delay array and loads the signal buffer into it. */

void MBASE::put_tap(int intap, float *Sig, int len)
{
	register double *tapdel = m_tapDelay;
	int tap = intap;
    while (tap >= m_tapsize)
        tap -= m_tapsize;
	for (int i = 0; i < len; ++i, tap++) {
	    if (tap >= m_tapsize)
		    tap = 0;
    	tapdel[tap] = Sig[i];
	}
}

/* ------------------------------------------------------------- mike_set --- */
/* mike_set determines the gain of each source vector based on its angle to
   the simulated microphones
 */
void MBASE::mike_set()
{
   double OmniFactor = 1.0 - MikePatternFactor;

   for (int i = 0; i < 2; ++i)
      for (int j = 0; j < m_paths; ++j)
         m_vectors[i][j].MikeAmp = 
		     OmniFactor + (MikePatternFactor * cos(m_vectors[i][j].Theta - MikeAngle));
}

/* ----------------------------------------------------------- earfil_set --- */
/* earfil_set is called by place to load coeffs for the fir filter bank
   used in ear, the binaural image filter.
*/
void
MBASE::earfil_set(int flag)
{
   for (int i = 0; i < 2; ++i)
      for (int j = 0; j < m_paths; ++j)
         setfir(m_vectors[i][j].Theta,
		        g_Nterms[j],
				flag,
				m_vectors[i][j].Fircoeffs,
				m_vectors[i][j].Firtaps);
}


/* -------------------------------------------------------------- tap_set --- */
/* tap_set determines the number of samps delay for each of the tap delay
   lines in move.  It compensates for the group phase delay in ear, and
   determines the max # of samps delay for loop purposes, later in move.
*/
long
MBASE::tap_set(int fir_flag)
{
   long int maxloc = 0;
   double   delay;

   extern const int g_Group_delay[13];        /* from firdata.h */

#ifdef debug
   printf("outlocs:\n");
#endif
   for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < m_paths; ++j) {
         delay = m_vectors[i][j].Rho / MACH1;                /* delay time */
         if (fir_flag)
            m_vectors[i][j].outloc = (long)(delay * SR - g_Group_delay[j] + 0.5);
         else
            m_vectors[i][j].outloc = (long)(delay * SR + 0.5);
         if (m_vectors[i][j].outloc > (float)maxloc)
            maxloc = (int)(m_vectors[i][j].outloc + 0.5);   /* max # of samps delay */
#ifdef debug
         printf("%ld ", m_vectors[i][j].outloc);
#endif
      }
#ifdef debug
      printf("\n");
#endif

   }
#ifdef debug
   printf("\n");
#endif
   return (maxloc);
}
