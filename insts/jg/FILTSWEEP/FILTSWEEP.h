#include <objlib.h>

#define MAXFILTS 5

class FILTSWEEP : public Instrument {
   int     inchan, skip, insamps, nfilts, do_balance;
   float   amp, pctleft, scale;
   float   *cfarray, cftabs[2], *bwarray, bwtabs[2];
   BiQuad  *filt[MAXFILTS];
   Balance *balancer;
   float   *in, *amparray, amptabs[2];

public:
   FILTSWEEP();
   virtual ~FILTSWEEP();
   int init(float *, short);
   int run();
};

