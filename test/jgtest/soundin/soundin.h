#include <objlib.h>

class soundin : public Instrument {
   int       inchan, skip;
   float     amp, pctleft;
   float     *amparray, amptabs[2];
   SoundIn   *infile;

public:
   soundin();
   virtual ~soundin();
   int init(float *, int);
   int run();
};

