extern "C" {
   #include "roomset.h"          /* only for NTAPS */
}

class ROOM : public Instrument {
   int    inchan, insamps, skip;
   int    nmax, jpoint, ipoint[NTAPS];
   float  lamp[NTAPS], ramp[NTAPS];
   float  amp;
   float  *in, *echo, amptabs[2];
   double *amparray;

public:
   ROOM();
   virtual ~ROOM();
   int init(double p[], int n_args);
   int run();
};

