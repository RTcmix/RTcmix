#include <objlib.h>

class JFIR : public Instrument {
   int     inchan, insamps, skip, bypass;
   float   amp, spread;
   float   *in, amptabs[2];
   double  *amparray;
   NZero   *filt;

public:
   JFIR();
   virtual ~JFIR();
   int init(double p[], int n_args);
   int run();
   void print_freq_response();
};

