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
   virtual int pre_init(float *, int);
   virtual int post_init(float *, int);
   virtual float process_sample(float, float);
   virtual void update_params(void);
   virtual const char *instname(void) { return "FOLLOWBUTTER"; }
};

