#include <objlib.h>

class JFIR : public Instrument {
   int     inchan, insamps, skip, bypass;
   float   amp, spread;
   float   *in, *amparray, amptabs[2];
   NZero   *filt;

public:
   JFIR();
   virtual ~JFIR();
   int init(double p[], int n_args);
   int run();
   void print_freq_response();
};

