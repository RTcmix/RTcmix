#include <Instrument.h>
#include <rtdefs.h>

class MOCKBEND : public Instrument {
   int    incount, inframe, skip, inchan, get_frame;
   int    in_frames_left;
   double increment, counter;
   float  amp, pctleft;
   float  newsig, oldsig, oldersig;
   double *amptable, *pitchtable;
   float  tabs[2], ptabs[2];
   float  *in;
public:
   MOCKBEND();
   virtual ~MOCKBEND();
   int init(double *, int);
   int run();
};

