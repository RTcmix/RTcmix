#include "revmodel.hpp"

class FREEVERB : public Instrument {
   bool     warn_roomsize, warn_predelay, warn_damp, warn_dry, warn_wet,
            warn_width;
   int      inchan, skip, branch, insamps;
   float    amp, ringdur, roomsize, predelay_time, max_roomsize,
            damp, dry, wet, width;
   float    *in, amptabs[2];
   double   *amparray;
   revmodel *rvb;

   void updateRvb(double p[]);
public:
   FREEVERB();
   virtual ~FREEVERB();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

