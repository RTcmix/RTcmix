#include <objlib.h>

class DECIMATE : public Instrument {
   bool     warn_bits, warn_cutoff, usefilt;
   int32_t  mask;
   int      nargs, inchan, skip, branch, bias, bits;
   float    preamp, postamp, cutoff, nyquist, pctleft;
   float    *in, amptabs[2];
   double   *amparray;
   Butter   *lpfilt;

   void changebits(int bits);
public:
   DECIMATE();
   virtual ~DECIMATE();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

