#include <rtdefs.h>
#include "elldefs.h"

class ELL : public Instrument {
   int     nargs, insamps, skip, branch, inchan, nsects;
   float   amp, pctleft, xnorm;
   float   *in, *amptable, amptabs[2];
   EllSect *es[MAXCHANS];

public:
   ELL();
   virtual ~ELL();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 3,
	kPan = 1 << 6
};
