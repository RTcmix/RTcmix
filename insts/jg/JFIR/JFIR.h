#include <objlib.h>

class JFIR : public Instrument {
   int     inchan, insamps, skip;
   float   amp, spread;
   float   *amparray, amptabs[2], *in;
   NZero   *filt;

public:
   JFIR();
   virtual ~JFIR();
   int init(float *, short);
   int run();
   void print_freq_response();
};

