#include <objlib.h>

class FLANGE : public Instrument {
   bool    flangetype_was_string;
   int     nargs, inchan, insamps, branch, flangetype;
   float   amp, resonance, moddepth, modrate, pctleft, wetdrymix, maxdelsamps;
   float   *in, amptabs[2];
   double  *amparray, *modtable;
   Butter  *filt;
   ZComb   *zcomb;
   ZNotch  *znotch;
   OscilL  *modoscil;
	bool ownModtable;

   int getFlangeType(bool trystring);
   void doupdate();
public:
   FLANGE();
   virtual ~FLANGE();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

// update flags (shift amount is pfield index)
enum {
	kResonance = 1 << 4,
	kModDepth = 1 << 6,
	kModRate = 1 << 7,
	kWetDry = 1 << 8,
	kType = 1 << 9,
	kPan = 1 << 11
};

