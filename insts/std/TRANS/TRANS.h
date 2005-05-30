#include <Instrument.h>
#include <rtdefs.h>

class TRANS : public Instrument {
   int    incount, inframe, branch, inchan, nargs;
   bool   getframe, fastUpdate;
   double _increment, counter, oneover_cpsoct10;
   float  amp, ampmult, pctleft, transp;
   float  newsig, oldsig, oldersig;
   double *amptable;
   float  *in, amptabs[2];

   void initamp(float dur, double p[], int ampindex, int ampgenslot);
   void doupdate();
public:
   TRANS();
   virtual ~TRANS();
   int init(double *, int);
   int configure();
   int run();
};

