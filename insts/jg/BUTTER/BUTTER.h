#include <objlib.h>

typedef enum {
   LowPass = 0,      // but note that user codes are 1-based (unlike EQ inst)
   HighPass,
   BandPass,
   BandReject,
   FiltInvalid
} FiltType;

#define MAXFILTS 30

class BUTTER : public Instrument {
   bool     do_balance, bypass, filttype_was_string;
   int      nargs, inchan, branch, skip, insamps, nfilts;
   float    amp, pctleft, cf, bw;
   double   *amparray, *cfarray, *bwarray;
   float    amptabs[2], cftabs[2], bwtabs[2];
   float    *in;
   FiltType type;
   Butter   *filt[MAXFILTS];
   Balance  *balancer;

   FiltType getFiltType(bool trystring);
   void doupdate();
public:
   BUTTER();
   virtual ~BUTTER();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

// update flags (shift amount is pfield index)
enum {
	kAmp = 1 << 3,
	kType = 1 << 4,
	kPan = 1 << 8,
	kBypass = 1 << 9,
	kFreq = 1 << 10,
	kBandwidth = 1 << 11
};

