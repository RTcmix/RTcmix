#include <objlib.h>

class REV : public Instrument {
   int     inchan, insamps, skip;
   float   amp, rvbtime, rvbpct;
   float   *amparray, amptabs[2], *in;
   Reverb  *reverb;

public:
   REV();
   virtual ~REV();
   int init(float *, short);
   int run();
};

