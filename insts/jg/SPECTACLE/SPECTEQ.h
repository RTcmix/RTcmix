#include "SPECTACLE_BASE.h"

class SPECTEQ : public SPECTACLE_BASE {

   float    *eqtable;

public:
   SPECTEQ();
   virtual ~SPECTEQ();

protected:
   virtual const char *instname() { return "SPECTEQ"; }
   virtual int pre_init(float *, int);
   virtual int post_init(float *, int);
   virtual void dump_anal_channels();
   virtual void modify_analysis();
};

