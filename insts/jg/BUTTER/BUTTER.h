#include <objlib.h>

typedef enum {
   LowPass = 1,
   HighPass = 2
} FiltType;

#define MAXFILTS 30

class BUTTER : public Instrument {
   int      inchan, skip, insamps, nfilts, do_balance;
   float    amp, pctleft, scale, reson;
   float    *cfarray, cftabs[2];
   FiltType type;
   Butter   *filt[MAXFILTS];
   Balance  *balancer;
   float    *in, *amparray, amptabs[2];

public:
   BUTTER();
   virtual ~BUTTER();
   int init(float *, int);
   int run();
};

