// MBASE.h -- base class for MPLACE and MMOVE

#ifndef _MBASE_H_
#define _MBASE_H_

#define MAXTERMS  33
#define NCOEFFS   512
#define MAX_INPUTS  4
#define AVERAGE_CHANS   -1           /* average input chans flag value */

#include <Instrument.h>
#include "msetup.h"

class MBASE : public Instrument {
public:
   MBASE();
   virtual ~MBASE();
   virtual int init(double *, int);
   virtual int configure();
   virtual int run();
protected:
   virtual int localInit(double *, int) = 0;
   virtual int finishInit(double *) = 0;
   virtual int updatePosition(int) = 0;
   virtual void get_tap(int, int, int, int) = 0;

   void setBufferSize(int size) { m_buffersize = size; }
   int getBufferSize(void) { return m_buffersize; }
   int getInput(int currentFrame, int frames);
   int alloc_delays(void);
   int alloc_firfilters(void);
   void get_lengths(long);
   void set_gains();
   void set_walls(float);
   void put_tap(int, float *, int);
   int  roomtrig(double, double, double, int);
   void rvb_reset(double *);
   void setair(double, int, double *, bool);
   void airfil_set(int);
   void earfil_set(int);
   long tap_set(int);
   void mike_set();
protected:
   int    m_inchan, insamps, m_binaural, m_tapsize, tapcount;
   int    m_cartflag, m_buffersize;
   int 	  m_branch;
   int	  m_paths;
   float  inamp, m_dur;
   float  amptabs[2], *in;
   double *amparray;
   double AIRCOEFFS[NCOEFFS];
   double m_dist, *m_tapDelay;
   double MikeAngle, MikePatternFactor;
   double Dimensions[5];
   AttenuationParams m_attenParams;
   // one of these per path per channel
   struct Vector {
   	double Rho;		// distance relative to listener
	double Theta;		// angle relative to listener
	double *Sig;	// current buffer for this path
	double MikeAmp;		// amp factor for microphone mode
	long outloc;		// index into tap delay
	double Airdata[3];	// history, coeffs for 1st order filter
	double Walldata[3];	// history, coeffs for 1st order filter
	double *Firtaps;
	double *Fircoeffs;
   };
   Vector m_vectors[2][13];
};

#undef min
#undef max
inline int min(int x, int y) { return x < y ? x : y; }
inline int max(int x, int y) { return x > y ? x : y; }
inline double min(double x, double y) { return x < y ? x : y; }
inline double max(double x, double y) { return x > y ? x : y; }

#endif	// _MBASE_H_
