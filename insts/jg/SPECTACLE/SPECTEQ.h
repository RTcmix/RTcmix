#include "SPECTACLE_BASE.h"

class SPECTEQ : public SPECTACLE_BASE {

   double   *eqtable;

public:
   SPECTEQ();
   virtual ~SPECTEQ();

protected:
   virtual const char *instname() { return "SPECTEQ"; }
   virtual int pre_init(double p[], int n_args);
   virtual int post_init(double p[], int n_args);
   virtual void dump_anal_channels();
   virtual void modify_analysis();
};

