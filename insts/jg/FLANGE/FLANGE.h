#include <objlib.h>

class FLANGE : public Instrument {
   int     inchan, insamps, skip, flangetype;
   float   amp, resonance, moddepth, modspeed, spread, wetdrymix, maxdelsamps;
   float   *amparray, amptabs[2], *in;
   Butter  *filt;
   ZComb   *zcomb;
   ZNotch  *znotch;
   OscilN  *modoscil;

public:
   FLANGE();
   virtual ~FLANGE();
   int init(float *, short);
   int run();
};

