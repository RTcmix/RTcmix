#include "FOLLOWER_BASE.h"

typedef enum {
   LowPass = 0,      // but note that user codes are 1-based (unlike EQ inst)
   HighPass,
   BandPass,
   BandReject,
   FiltInvalid
} FiltType;

#define MAXFILTS 30

class FOLLOWBUTTER : public FOLLOWER_BASE {
   bool     filttype_was_string;
   int      nfilts;
   float    mincf, maxcf, cfdiff, cf, bw, nyquist;
   FiltType type;
   Butter   *filt[MAXFILTS];
   TableL   *bwtable;

   FiltType getFiltType(bool trystring);
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

