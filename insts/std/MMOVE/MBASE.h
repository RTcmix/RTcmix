// MBASE.h -- base class for MPLACE and MMOVE

#ifndef _MBASE_H_
#define _MBASE_H_

#define MAXTERMS  33
#define NCOEFFS   512
#define MAX_INPUTS  4
#define AVERAGE_CHANS   -1           /* average input chans flag value */
#define SQRT_TWO 1.4142136
#define SIG_THRESH 100000000.0

#include <stdio.h>
#include <Instrument.h>
#include <vector>
#include <memory>
#include "msetup.h"

inline void PrintInput(float *sig, int len)
{
    for (int i = 0; i < len; i++)
        if (sig[i] != 0.0)
            printf("sig[%d] = %f\n", i, sig[i]);
    printf("\n");
}

inline void PrintOutput(float *sig, int len, int chans, double threshold = 0.0)
{
    const int samps = len * chans;
    bool doPrint = false;
    int count = 0;
    for (int i = 0; i < samps; i += chans) {
        if (sig[i] > threshold || sig[i] < -threshold)
            doPrint = (count < 5);
        if (doPrint) {
            printf("sig[%d] = %f\n", i / chans, sig[i]);
            ++count;
        }
    }
    if (count > 0) { printf("..."); }
    printf("\n");
}

inline void PrintSig(double *sig, int len, double threshold = 0.0)
{
    int count = 0;
    for (int i = 0; i < len; i++) {
        if (count < 5 && (sig[i] > threshold || sig[i] < -threshold)) {
            printf("sig[%d] = %f\n", i, sig[i]);
            ++count;
        }
    }
    if (count > 0) { printf("..."); }
    printf("\n");
}

class MBASE : public Instrument {
public:
   MBASE(int chans, int paths);
   virtual ~MBASE();
   virtual int init(double *, int);
   virtual int configure();
protected:
    virtual int checkOutputChannelCount() = 0;
    virtual int localInit(double *, int) = 0;
    virtual int finishInit(double *) = 0;
    virtual int updatePosition(int) = 0;
    virtual void get_tap(int, int, int, int) = 0;
protected:
   virtual int alloc_vectors() = 0;
   void setBufferSize(int size) { m_buffersize = size; }
   int getBufferSize() const { return m_buffersize; }
   int getInput(int currentFrame, int frames);
   bool binaural() const { return (!UseMikes && m_dist < 0.8 && m_dist != 0.0); }
   int alloc_delays();
   void get_lengths(long);
   void set_gains();
   void set_walls(float);
   void put_tap(int, float *, int);
   int  roomtrig(double A, double B, double H, double Yoffsets[], int useCartesian);
   void rvb_reset();
   void setair(double, int, double *, bool);
   void airfil_set(int);
   long tap_set(int);
   void mike_set();
protected:
   int    m_inchan, insamps, m_tapsize, tapcount;
   int    m_cartflag, m_buffersize;
   int 	  m_branch;
   int	  m_chans, m_paths;
   float  inamp, m_dur;
   float  amptabs[2], *in;
   double **m_mixbufs;
   double *amparray;
   double AIRCOEFFS[NCOEFFS];
   double m_dist, *m_tapDelay;
   double m_absorbFactor;
   int UseMikes;
   double MikeAngle, MikePatternFactor;
   double Dimensions[5];
   AttenuationParams m_attenParams;
   // one of these per path per channel
   struct Vector {
       Vector() : Sig(NULL) {}
       virtual ~Vector();
       void configure(int bufSize);
       void setWallFilter(double SR, double cf);
       void resetFilters();
       virtual void runFilters(int currentSamp, int len, int pathIndex);
       double Rho;		// distance relative to listener
       double Theta;		// angle relative to listener
       double *Sig;	// current buffer for this path
       double MikeAmp;		// amp factor for microphone mode
       long outloc;		// index into tap delay
       double Airdata[3];	// history, coeffs for 1st order filter
       double Walldata[3];	// history, coeffs for 1st order filter
   };
   typedef std::shared_ptr<Vector> SVector;
   typedef std::vector<SVector> VecEntry;
   std::vector<VecEntry> m_vectors;
};

#undef min
#undef max
inline int min(int x, int y) { return x < y ? x : y; }
inline int max(int x, int y) { return x > y ? x : y; }
inline double min(double x, double y) { return x < y ? x : y; }
inline double max(double x, double y) { return x > y ? x : y; }

#endif	// _MBASE_H_
