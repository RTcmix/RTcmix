#include <objlib.h>

typedef enum {
   AS = 0,
   FM,
   SAMP
} OscType;

class JGRAN : public Instrument {
   int      randomize_phase, skip, branch, krate, gsampcount, curgsamp;
   float    amp, aamp, ranphase, envoscil_freq;
   float    next_graindur, graindur;
   float    next_grainamp, grainamp;
   float    next_carfreq, carfreq;
   float    next_modfreq, modfreq;
   float    next_moddepth, moddepth;
   float    next_grainpan, grainpan[2];
   OscType  osctype;
   TableL   *amp_table;
   TableL   *modmult_table, *modindex_table;
   TableL   *minfreq_table, *maxfreq_table;
   TableL   *minspeed_table, *maxspeed_table;
   TableL   *minintens_table, *maxintens_table;
   TableL   *density_table, *pan_table, *panvar_table;
   OscilN   *car_oscil, *mod_oscil, *grainenv_oscil;
   Noise    *durnoi, *freqnoi, *pannoi, *ampnoi, *phasenoi;

public:
   JGRAN();
   virtual ~JGRAN();
   int init(double p[], int n_args);
   int run();
};

