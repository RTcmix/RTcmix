#include <Instrument.h>
#include <rtdefs.h>

class TRANS : public Instrument {
   int    incount, inframe, skip, branch, inchan, get_frame;
   int    in_frames_left;
   double increment, counter;
   float  amp, aamp, pctleft;
   float  newsig, oldsig, oldersig;
   float  *amptable, tabs[2];
   float  *in;
public:
   TRANS();
   virtual ~TRANS();
   int init(double *, int);
   int run();
};

