#include "FOLLOWER_BASE.h"

typedef enum {
   LowPass = 1,
   HighPass = 2,
   BandPass = 3,
   BandReject = 4
} FiltType;

#define MAXFILTS 30

class FOLLOWBUTTER : public FOLLOWER_BASE {
   int      nfilts;
   float    mincf, maxcf, cfdiff, curcf, curbw;
   FiltType type;
   Butter   *filt[MAXFILTS];
   TableL   *bwtable;
public:
   FOLLOWBUTTER();
   virtual ~FOLLOWBUTTER();
protected:
   virtual int pre_init(double p[], int n_args);
   virtual int post_init(double p[], int n_args);
   virtual float process_sample(float, float);
   virtual void update_params(double p[]);
   virtual const char *instname() { return "FOLLOWBUTTER"; }
};

