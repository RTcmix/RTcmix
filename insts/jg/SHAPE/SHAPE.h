#include <objlib.h>

class SHAPE : public Instrument {
   int      inchan, skip, branch;
   float    amp, aamp, index, min_index, max_index, norm_index, pctleft;
   float    *in;
   TableL   *amp_table, *index_table;
   WavShape *shaper, *ampnorm;
   DCBlock  *dcblocker;

public:
   SHAPE();
   virtual ~SHAPE();
   int init(float *, int);
   int run();
};

