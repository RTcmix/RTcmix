#include <rtdefs.h>
#include "elldefs.h"

class ELL : public Instrument {
   int     insamps, skip, inchan, nsects;
   float   amp, spread, xnorm;
   float   *in, *amptable, amptabs[2];
   EllSect *es[MAXCHANS];

public:
   ELL();
   virtual ~ELL();
   int init(float *, short);
   int run();
};

