#include <objlib.h>

class REV : public Instrument {
   int     inchan, insamps, skip, branch;
   float   amp, *in, amptabs[2];
   double  *amparray;
   Reverb  *reverb;

public:
   REV();
   virtual ~REV();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

// update flags (shift amount is pfield number)
enum {
   kAmp = 1 << 3,
   kWetPercent = 1 << 6
};

