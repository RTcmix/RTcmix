#include <objlib.h>

class EQ : public Instrument {
   int         inchan, skip, branch, bypass, insamps;
   float       amp, aamp, pctleft;
   float       *in;
   float       freq, Q, gain, prev_freq, prev_Q, prev_gain;
   TableL      *amp_table, *freq_table, *q_table, *gain_table;
   Equalizer   *eq;
   EQType      eqtype;

public:
   EQ();
   virtual ~EQ();
   int init(double p[], int n_args);
   int run();
};

