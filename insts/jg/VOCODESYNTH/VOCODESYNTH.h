#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>
#include <assert.h>
#include <objlib.h>

#ifdef DEBUG
   #define DPRINT(msg)                    printf((msg))
   #define DPRINT1(msg, arg)              printf((msg), (arg))
   #define DPRINT2(msg, arg1, arg2)       printf((msg), (arg1), (arg2))
#else
   #define DPRINT(msg)
   #define DPRINT1(msg, arg)
   #define DPRINT2(msg, arg1, arg2)
#endif

#define MAXOSC 200

typedef enum {
   belowThreshold,
   aboveThreshold
} PowerState;

class VOCODESYNTH : public Instrument {
   int         skip, numbands, branch, inchan, insamps, inringdown;
   float       amp, aamp, pctleft, hipass_mod_amp, smoothness;
   float       threshold, attack_rate, release_rate;
   float       *in;
   float       carfreq[MAXOSC], lastpower[MAXOSC];
   double      *car_wavetable, *scaletable;
   Butter      *modulator_filt[MAXOSC], *hipassmod;
   OscilN      *carrier_osc[MAXOSC];
   TableL      *amptable;
   RMS         *gauge[MAXOSC];
   OnePole     *smoother[MAXOSC];
   Envelope    *envelope[MAXOSC];
   PowerState  state[MAXOSC];

public:
   VOCODESYNTH();
   virtual ~VOCODESYNTH();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

