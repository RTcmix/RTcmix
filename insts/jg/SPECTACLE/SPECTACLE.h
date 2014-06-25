#include "SPECTACLE_BASE.h"

class SPECTACLE : public SPECTACLE_BASE {

   double   *eqtable, *deltimetable, *feedbacktable;
   DLineN   *phase_delay[MAXFFTLEN / 2], *mag_delay[MAXFFTLEN / 2];

public:
   SPECTACLE();
   virtual ~SPECTACLE();

protected:
   virtual const char *instname() { return "SPECTACLE"; }
   virtual int pre_init(double p[], int n_args);
   virtual int post_init(double p[], int n_args);
   virtual void dump_anal_channels();
   virtual void modify_analysis();
};

