#include <Instrument.h>
#include <rtdefs.h>

class TRANS3 : public Instrument {
   int    incount, inframe, skip, inchan, get_frame;
   int    in_frames_left;
   double increment, counter;
   float  amp, pctleft;
   float  newestsig, newsig, oldsig, oldersig;
   double *amptable;
   float  *in, tabs[2];
public:
   TRANS3();
   virtual ~TRANS3();
   int init(double *, int);
   int run();
};

