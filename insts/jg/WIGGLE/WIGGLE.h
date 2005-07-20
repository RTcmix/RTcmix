#include <objlib.h>

typedef enum {
   NoModOsc = 0,
   CarPercent = 1,
   ModIndex = 2
} DepthType;

typedef enum {
   NoFilter = 0,
   LowPass = 1,
   HighPass = 2
} FiltType;

#define MAXFILTS 50

class WIGGLE : public Instrument {
   bool     do_balance;
   int      depth_type, nfilts, branch;
   float    amp, pan, cpsoct10, car_freq_raw, base_car_freq, car_freq;
   float    mod_freq, mod_depth, nyquist, cf_raw;
   FiltType filter_type;
   Butter   *filt[MAXFILTS];
   Balance  *balancer;
   OscilL   *carrier;
   OscilL   *modulator;
   double   *carwave_array, *modwave_array;
   TableL   *amp_table;
   TableN   *cargliss_table;
   TableL   *modfreq_table;
   TableL   *moddepth_table;
   TableL   *filtcf_table;
   TableN   *pan_table;

   DepthType getDepthType(double pval);
   FiltType getFiltType(double pval);
   void doupdate();
public:
   WIGGLE();
   virtual ~WIGGLE();
   virtual int init(double p[], int n_args);
   virtual int run();
};

