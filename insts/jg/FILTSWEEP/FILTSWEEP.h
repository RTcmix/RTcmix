#include <objlib.h>

#define MAXFILTS 5

class FILTSWEEP : public Instrument {
   int     inchan, skip, insamps, nfilts, do_balance;
   float   amp, pctleft, scale;
   float   *in;
   float   amptabs[2], cftabs[2], bwtabs[2];
   double  *amparray, *cfarray, *bwarray;
   BiQuad  *filt[MAXFILTS];
   Balance *balancer;

public:
   FILTSWEEP();
   virtual ~FILTSWEEP();
   int init(double p[], int n_args);
   int run();
};

