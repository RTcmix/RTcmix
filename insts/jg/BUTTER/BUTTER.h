#include <objlib.h>

typedef enum {
   LowPass = 1,
   HighPass = 2,
   BandPass = 3,
   BandReject = 4
} FiltType;

#define MAXFILTS 30

class BUTTER : public Instrument {
   int      inchan, branch, skip, insamps, nfilts, do_balance, bypass;
   float    amp, aamp, pctleft, scale, reson, curcf, curbw;
   double   *amparray, *cfarray, *bwarray;
   float    amptabs[2], cftabs[2], bwtabs[2];
   float    *in;
   FiltType type;
   Butter   *filt[MAXFILTS];
   Balance  *balancer;

public:
   BUTTER();
   virtual ~BUTTER();
   int init(double p[], int n_args);
   int run();
};

