#include <objlib.h>

#define MAXFILTS 200

class VOCODE2 : public Instrument {
   int       skip, numfilts, branch;
   float     amp, aamp, pctleft, noise_amp, hipass_mod_amp;
   float     *in, *amparray, amptabs[2];
   SubNoiseL *noise;
   Butter    *modulator_filt[MAXFILTS], *carrier_filt[MAXFILTS], *hipassmod;
   Balance   *balancer[MAXFILTS];

public:
   VOCODE2();
   virtual ~VOCODE2();
   int init(float *, int);
   int run();
};

