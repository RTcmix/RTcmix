#include <objlib.h>

typedef enum {
   SoftClip = 1,
   Tube = 2
} DistortType;

class DISTORT : public Instrument {
   bool        usefilt, bypass;
   int         nargs, inchan, branch, skip;
   float       amp, gain, cutoff, pctleft;
   float       *in;
   DistortType type;
   Butter      *filt;
   TableL      *amptable;

   DistortType getDistortType(double pval);
public:
   DISTORT();
   virtual ~DISTORT();
   virtual int init(double p[], int n_args);
   virtual int configure();
   float distort(float, float);
   virtual int run();
};

// update flags (shift amount is pfield number)
enum {
   kAmp = 1 << 3,
   kGain = 1 << 5,
   kFiltCF = 1 << 6,
   kPan = 1 << 8,
   kBypass = 1 << 9
};

