extern "C" {
   #include "roomset.h"          /* only for NTAPS */
}

class ROOM : public Instrument {
   int    inchan, insamps, skip;
   int    nmax, jpoint, ipoint[NTAPS];
   float  lamp[NTAPS], ramp[NTAPS];
   float  amp;
   float  *echo, *amparray, amptabs[2], *in;

public:
   ROOM();
   virtual ~ROOM();
   int init(float *, short);
   int run();
};

