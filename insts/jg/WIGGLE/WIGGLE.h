#include <objlib.h>

typedef enum {
   CarPercent = 0,
   ModIndex = 1
} DepthType;

typedef enum {
   NoFilter = 0,
   LowPass = 1,
   HighPass = 2
} FiltType;

#define MAXFILTS 50

class WIGGLE : public Instrument {
   int      inchan, skip, depth_type, nfilts, do_balance, branch;
   float    amp, aamp, pctleft, base_freq, cpsoct10;
   float    car_freq, car_gliss, mod_freq, mod_depth;
   FiltType filter_type;
   Butter   *filt[MAXFILTS];
   Balance  *balancer;
   OscilL   *carrier;
   OscilL   *modulator;
   float    *amp_array, *carwave_array, *cargliss_array, *modwave_array;
   float    *modfreq_array, *moddepth_array, *filtcf_array, *pan_array;
   TableL   *amp_table;
   TableN   *cargliss_table;
   TableL   *modfreq_table;
   TableL   *moddepth_table;
   TableL   *filtcf_table;
   TableN   *pan_table;

public:
   WIGGLE();
   virtual ~WIGGLE();
   int init(float *, int);
   int run();
};

