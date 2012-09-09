// BASE.h -- base class for PLACE and MOVE

#define MAXTERMS  33       // also defined in firdata.h
#define NPRIMES   5000
#define NCOEFFS   512
#define BUFLEN	  256
#define MAX_INPUTS  4
#define AVERAGE_CHANS   -1           /* average input chans flag value */

#include <Instrument.h>

class BASE : public Instrument {
public:
   BASE();
   virtual ~BASE();
   virtual int init(double *, int);
   virtual int configure();
   virtual int run();
protected:
   virtual int localInit(double *, int) = 0;
   virtual int finishInit(double, double *) = 0;
   virtual int updatePosition(int) = 0;
   virtual void get_tap(int, int, int, int) = 0;

   void setBufferSize(int size) { m_buffersize = size; }
   int getBufferSize(void) { return m_buffersize; }
   int getInput(int currentSample, int frames);
   void wire_matrix(double [12][12]);
   int alloc_delays(void);
   int alloc_firfilters(void);
   void get_lengths(long);
   void set_gains(float);
   void set_walls(float);
   void set_allpass(void);
   void set_random(void);
   void put_tap(int, float *, int);
   int  roomtrig(double, double, double, int);
   void rvb_reset(double *);
   void setair(double, int, double *);
   void airfil_set(int);
   void earfil_set(int);
   long tap_set(int);
   void mike_set();
   void RVB(double *, double *, long);
   void matrix_mix();
   // static methods
   static void get_primes(int x, int p[]);
protected:
   int    m_inchan, insamps, m_binaural, m_tapsize, tapcount;
   int    cartflag, rvbdelsize, m_buffersize, m_branch;
   float  inamp, m_dur, m_rvbamp;
   float  amptabs[2], *in;
   double *amparray;
   double AIRCOEFFS[NCOEFFS];
   double m_dist, *m_tapDelay;
   double MikeAngle, MikePatternFactor;
   double Dimensions[5];
   struct ReverbPatch {
   	int incount;
   	double *inptrs[MAX_INPUTS];
	double gains[MAX_INPUTS];
	double *outptr;
   } ReverbPatches[12];
   double Nsdelay[2][6];
   // one of these per path per channel
   struct Vector {
   	double Rho;		// distance relative to listener
	double Theta;		// angle relative to listener
	double Sig[BUFLEN];	// current buffer for this path
	double MikeAmp;		// amp factor for microphone mode
	long outloc;		// index into tap delay
	double Airdata[3];	// history, coeffs for 1st order filter
	double Walldata[3];	// history, coeffs for 1st order filter
	double *Firtaps;
	double *Fircoeffs;
   };
   Vector m_vectors[2][13];
   
   struct ReverbData {
	double delin;
   	double Rand_info[6];
	double *Rvb_del;
	int deltap;
	double Rvb_air[3];
	double delout;
   };
   ReverbData m_rvbData[2][6];
   double Allpass_del[2][502];
   int allpassTap[2];
   double m_rvbPast[2];	// For hi-pass filter
   // static data
   static int    primes[NPRIMES + 2];
   static AtomicInt primes_gotten;
};

#undef min
#undef max
inline int min(int x, int y) { return x < y ? x : y; }
inline int max(int x, int y) { return x > y ? x : y; }
inline double min(double x, double y) { return x < y ? x : y; }
inline double max(double x, double y) { return x > y ? x : y; }

