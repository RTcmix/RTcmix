#include <objlib.h>

class JFIR : public Instrument {
   bool    bypass;
   int     nargs, inchan, insamps, skip, branch;
   float   amp, pctleft;
   float   *in, amptabs[2];
   double  *amparray;
   NZero   *filt;

   void doupdate();
public:
   JFIR();
   virtual ~JFIR();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
   void print_freq_response();
};

// update flags (shift amount is pfield index)
enum {
	kAmp = 1 << 3,
	kPan = 1 << 6,
	kBypass = 1 << 7
};

