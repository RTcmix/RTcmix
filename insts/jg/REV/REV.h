#include <objlib.h>

class REV : public Instrument {
   int     inchan, insamps, branch;
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

