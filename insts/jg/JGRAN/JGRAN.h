#include <objlib.h>

typedef enum {
   AS = 0,
   FM
} OscType;

class JGRAN : public Instrument {
   bool     randomize_phase;
   int      nargs, skip, branch, krate, gsampcount, curgsamp;
   double   amp, ranphase, envoscil_freq;
   double   next_graindur, graindur;
   double   next_grainamp, grainamp;
   double   next_carfreq, carfreq;
   double   next_modfreq, modfreq;
   double   next_moddepth, moddepth;
   double   next_grainpan, grainpan[2];
   OscType  osctype;
   TableL   *amp_table;
   TableL   *modmult_table, *modindex_table;
   TableL   *minfreq_table, *maxfreq_table;
   TableL   *minspeed_table, *maxspeed_table;
   TableL   *minintens_table, *maxintens_table;
   TableL   *density_table, *pan_table, *panvar_table;
   OscilL   *car_oscil, *mod_oscil, *grainenv_oscil;
   JGNoise    *durnoi, *freqnoi, *pannoi, *ampnoi, *phasenoi;

   void doupdate();
public:
   JGRAN();
   virtual ~JGRAN();
   virtual int init(double p[], int n_args);
   virtual int run();
};

