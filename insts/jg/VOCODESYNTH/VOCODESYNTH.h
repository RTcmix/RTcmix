#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <mixerr.h>
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
   float       *in, *car_wavetable, *scaletable;
   float       carfreq[MAXOSC], lastpower[MAXOSC];
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
   int init(double p[], int n_args);
   int run();
};

