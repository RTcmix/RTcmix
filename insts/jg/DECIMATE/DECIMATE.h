#include <objlib.h>

class DECIMATE : public Instrument {
   int      inchan, skip, branch, mask, bias;
   float    amp, aamp, pctleft;
   float    *in, *amparray, amptabs[2];
   Butter   *lpfilt;

public:
   DECIMATE();
   virtual ~DECIMATE();
   int init(double p[], int n_args);
   int run();
};

