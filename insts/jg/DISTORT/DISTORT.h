#include <objlib.h>

typedef enum {
   SoftClip = 1,
   Tube = 2
} DistortType;

class DISTORT : public Instrument {
   int         inchan, branch, skip, bypass;
   float       amp, aamp, gain, pctleft;
   float       *in;
   DistortType type;
   Butter      *filt;
   TableL      *amptable;

public:
   DISTORT();
   virtual ~DISTORT();
   int init(double p[], int n_args);
   float distort(float, float);
   int run();
};

