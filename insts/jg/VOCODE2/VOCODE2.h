#include <objlib.h>

#define MAXFILTS 200

class VOCODE2 : public Instrument {
   int       nargs, skip, numfilts, branch;
   float     amp, pctleft, noise_amp, hipass_mod_amp;
   float     *in, amptabs[2];
   double    *amparray;
   SubNoiseL *noise;
   Butter    *modulator_filt[MAXFILTS], *carrier_filt[MAXFILTS], *hipassmod;
   Balance   *balancer[MAXFILTS];

   void doupdate();
public:
   VOCODE2();
   virtual ~VOCODE2();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 3,
	kNoiseAmp = 1 << 12,
	kPan = 1 << 14
};

