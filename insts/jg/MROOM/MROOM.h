extern "C" {
   #include "timeset.h"
}

#define NTAPS 10

class MROOM : public Instrument {
   int    inchan, insamps, skip, quantskip;
   int    deltabs[2];
   float  ovamp, xdim, ydim, reflect, innerwidth;
   float  del[NTAPS], amp[NTAPS];
   float  timepts[TIME_ARRAY_SIZE];
   float  xvals[TIME_ARRAY_SIZE], yvals[TIME_ARRAY_SIZE];
   float  xpos[POS_ARRAY_SIZE], ypos[POS_ARRAY_SIZE];
   float  amptabs[2], xpostabs[2], ypostabs[2];
   float  *delayline, *rvbarrayl, *rvbarrayr, *amparray, *in;

public:
   MROOM();
   virtual ~MROOM();
   int init(float *, short);
   int run();
private:
   void traject(int);
   float distndelset(float, float, float, float, float, float);
};

