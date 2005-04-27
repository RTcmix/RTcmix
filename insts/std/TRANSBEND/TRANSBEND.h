#include <Instrument.h>
#include <rtdefs.h>

class TRANSBEND : public Instrument {
   int    incount, inframe, skip, inchan, get_frame;
   int    in_frames_left;
   double _increment, counter;
   float  amp, pctleft;
   float  newsig, oldsig, oldersig;
   float  tabs[2], ptabs[2];
   double *amptable, *pitchtable;
   float  *in;
public:
   TRANSBEND();
   virtual ~TRANSBEND();
   int init(double *, int);
   int configure();
   int run();
};

