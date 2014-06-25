#include <objlib.h>

class SHAPE : public Instrument {
   int      nargs, inchan, skip, branch;
   float    amp, index, min_index, max_index, norm_index, pctleft;
   float    *in;
   TableL   *amp_table, *index_table;
   WavShape *shaper, *ampnorm;
   DCBlock  *dcblocker;

   void doupdate();
public:
   SHAPE();
   virtual ~SHAPE();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 3,
	kMinIndex = 1 << 4,
	kMaxIndex = 1 << 5,
	kPan = 1 << 8,
	kIndex = 1 << 10
};

