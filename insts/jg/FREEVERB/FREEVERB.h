#include "revmodel.hpp"

class FREEVERB : public Instrument {
   bool     warn_roomsize, warn_predelay, warn_damp, warn_dry, warn_wet,
            warn_width;
   int      branch, insamps;
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

// update flags (shift amount is pfield index)
enum {
	kRoomSize = 1 << 4,
	kPreDelay = 1 << 5,
	kDamp = 1 << 7,
	kDry = 1 << 8,
	kWet = 1 << 9,
	kWidth = 1 << 10
};

