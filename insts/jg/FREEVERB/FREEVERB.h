#include "revmodel.hpp"

class FREEVERB : public Instrument {
   int      inchan, skip, branch, insamps;
   float    amp, aamp, ringdur;
   float    *in, *amparray, amptabs[2];
   revmodel *rvb;

public:
   FREEVERB();
   virtual ~FREEVERB();
   int init(double p[], int n_args);
   int run();
};

