#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>
#include <assert.h>

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

class Butter;
class Envelope;
class JGOnePole;
class RMS;
class TableL;
class Ooscili;

typedef enum {
   belowThreshold,
   aboveThreshold
} PowerState;

class VOCODESYNTH : public Instrument {
   int         nargs, numbands, branch, inchan, insamps, inringdown;
   float       amp, pan, hipass_mod_amp, smoothness;
   float       threshold, attack_rate, release_rate;
   float       *in;
   float       lastpower[MAXOSC];
   double      *car_wavetable, *scaletable;
   Butter      *modulator_filt[MAXOSC], *hipassmod;
   Ooscili     *carrier_osc[MAXOSC];
   TableL      *amptable;
   RMS         *gauge[MAXOSC];
   JGOnePole     *smoother[MAXOSC];
   Envelope    *envelope[MAXOSC];
   PowerState  state[MAXOSC];

   void doupdate();
public:
   VOCODESYNTH();
   virtual ~VOCODESYNTH();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 3,
	kPan = 1 << 17
};

