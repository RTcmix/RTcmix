#include <Instrument.h>
#include <rtdefs.h>

class TRANS : public Instrument {
   int    incount, inframe, skip, inchan, getflag, first_time;
   double increment, counter;
   float  amp, pctleft;
   float  newsig, oldsig, oldersig;
   float  *amptable, tabs[2];
   float  *in, *out;
public:
   TRANS();
   virtual ~TRANS();
   int init(float *, short);
   int run();
};

