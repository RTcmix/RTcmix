extern "C" {
   #include "roomset.h"          /* only for NTAPS */
}

class ROOM : public Instrument {
   int    inchan, insamps, skip;
   int    nmax, jpoint, ipoint[NTAPS];
   float  lamp[NTAPS], ramp[NTAPS];
   float  amp;
   float  *in, *echo, *amparray, amptabs[2];

public:
   ROOM();
   virtual ~ROOM();
   int init(float *, int);
   int run();
};

