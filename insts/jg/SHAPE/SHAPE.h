#include <objlib.h>

class SHAPE : public Instrument {
   int      inchan, skip, branch;
   float    amp, aamp, pctleft;
   float    *in, *amparray, amptabs[2];
   float    *transfer_func;
   WavShape *shaper;

public:
   SHAPE();
   virtual ~SHAPE();
   int init(float *, int);
   int run();
};

