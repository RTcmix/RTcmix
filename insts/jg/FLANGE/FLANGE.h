#include <objlib.h>

class FLANGE : public Instrument {
   int     inchan, insamps, skip, flangetype;
   float   amp, resonance, moddepth, modspeed, spread, wetdrymix, maxdelsamps;
   float   *in, amptabs[2];
   double  *amparray;
   Butter  *filt;
   ZComb   *zcomb;
   ZNotch  *znotch;
   OscilN  *modoscil;

public:
   FLANGE();
   virtual ~FLANGE();
   int init(double p[], int n_args);
   int run();
};

