#include <objlib.h>

class EQ : public Instrument {
   bool        bypass, eqtype_was_string;
   int         nargs, inchan, skip, branch, insamps;
   float       amp, pctleft, freq, Q, gain;
   float       *in;
   TableL      *amp_table, *freq_table, *q_table, *gain_table;
   Equalizer   *eq;
   EQType      eqtype;

   EQType getEQType(bool trystring);
   void doupdate();
public:
   EQ();
   virtual ~EQ();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

// update flags (shift amount is pfield index)
enum {
	kAmp = 1 << 3,
	kType = 1 << 4,
	kPan = 1 << 6,
	kBypass = 1 << 7,
	kFreq = 1 << 8,
	kQ = 1 << 9,
	kGain = 1 << 10
};

