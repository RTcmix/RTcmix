#include <rtdefs.h>
#include "elldefs.h"

class ELL : public Instrument {
   int     insamps, skip, inchan, nsects;
   float   amp, pctleft, xnorm;
   float   *in, *amptable, amptabs[2];
   EllSect *es[MAXCHANS];

public:
   ELL();
   virtual ~ELL();
   int init(float *, int);
   int run();
};

