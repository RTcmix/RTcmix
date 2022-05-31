#include <Instrument.h>
#include <rtdefs.h>

class TRANS3 : public Instrument {
   bool   _useRatio;
   int    incount, inframe, branch, inchan, nargs;
   bool   getframe;
   double _increment, counter, oneover_cpsoct10;
   float  amp, pctleft, transp;
   float  newestsig, newsig, oldsig, oldersig;
   double *amptable;
   float  *in, amptabs[2];

   void doupdate();
public:
   TRANS3();
   virtual ~TRANS3();
   int init(double *, int);
   int configure();
   int run();
protected:
   TRANS3(bool inUseRatio);
   void clear();
};

class RTRANS3 : public TRANS3 {
public:
    RTRANS3();
    virtual ~RTRANS3();
};

