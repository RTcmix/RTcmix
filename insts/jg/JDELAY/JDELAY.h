#include <Ougens.h>

class JDELAY : public Instrument {
   bool    dcblock, prefadersend, usefilt, warn_deltime;
   int     nargs, insamps, skip, branch, inchan;
   float   amp, regen, cutoff, percent_wet, pctleft;
   float   prev_in, prev_out;
   float   *in, amptabs[2];
   double  delsamps, *amptable, tonedata[3];
   Odelayi *delay;

   void doupdate();
public:
   JDELAY();
   virtual ~JDELAY();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

// update flags (shift amount is pfield number)
enum {
   kAmp = 1 << 3,
   kDelTime = 1 << 4,
   kDelRegen = 1 << 5,
   kCutoff = 1 << 7,
   kWetPercent = 1 << 8,
   kPan = 1 << 10
};

