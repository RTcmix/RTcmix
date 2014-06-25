#include <Instrument.h>
#include <rtdefs.h>

class MOCKBEND : public Instrument {
   int    incount, inframe, skip, branch, inchan, get_frame;
   int    in_frames_left;
   double incr, counter;
   float  amp, aamp, pctleft;
   float  newsig, oldsig, oldersig;
   double *amptable, *pitchtable;
   float  tabs[2], ptabs[2];
   float  *in;
public:
   MOCKBEND();
   virtual ~MOCKBEND();
   virtual int init(double *, int);
   virtual int configure();
   virtual int run();
};

