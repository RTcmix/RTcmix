extern "C" {
   #include "roomset.h"          /* only for NTAPS */
}

class ROOM : public Instrument {
   int    inchan, insamps, skip, branch;
   int    nmax, jpoint, ipoint[NTAPS];
   float  lamp[NTAPS], ramp[NTAPS];
   float  amp, aamp;
   float  *in, *echo, amptabs[2];
   double *amparray;

public:
   ROOM();
   virtual ~ROOM();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

