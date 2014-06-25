extern "C" {
   #include "timeset.h"
}

#define NTAPS 10

class MROOM : public Instrument {
   int    inchan, insamps, skip, branch, quantbranch, quantskip;
   int    deltabs[3];
   float  aamp, ovamp, xdim, ydim, reflect, innerwidth;
   float  del[NTAPS], amp[NTAPS];
   float  timepts[TIME_ARRAY_SIZE];
   float  xvals[TIME_ARRAY_SIZE], yvals[TIME_ARRAY_SIZE];
   double xpos[POS_ARRAY_SIZE], ypos[POS_ARRAY_SIZE];
   float  amptabs[2], xpostabs[2], ypostabs[2];
   float  *in, *delayline, *rvbarrayl, *rvbarrayr;
   double *amparray;

public:
   MROOM();
   virtual ~MROOM();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
private:
   void traject(int);
   float distndelset(float, float, float, float, float, float);
};

