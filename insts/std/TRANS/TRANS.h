#include <Instrument.h>
#include <rtdefs.h>

class TRANS : public Instrument {
   int    incount, inframe, skip, branch, inchan, nargs;
   bool   getframe;
   double increment, counter, oneover_cpsoct10;
   float  amp, pctleft, transp;
   float  newsig, oldsig, oldersig;
   double *amptable;
   float  *in, tabs[2];

   void doupdate();
public:
   TRANS();
   virtual ~TRANS();
   int init(double *, int);
   int configure();
   int run();
};

