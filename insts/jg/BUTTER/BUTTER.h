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
   float    *cfarray, cftabs[2];
   float    *bwarray, bwtabs[2];
   FiltType type;
   Butter   *filt[MAXFILTS];
   Balance  *balancer;
   float    *in, *amparray, amptabs[2];

public:
   BUTTER();
   virtual ~BUTTER();
   int init(double p[], int n_args);
   int run();
};

