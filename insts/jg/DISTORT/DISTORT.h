class Odistort;
class Butter;
class TableL;

class DISTORT : public Instrument {

public:
   DISTORT();
   virtual ~DISTORT();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();

private:
	void doupdate();

   bool     usefilt, bypass;
   int      nargs, inchan, branch, type;
   float    amp, gain, cutoff, pctleft, param;
   float    *in;
	Odistort	*distort;
   Butter   *filt;
   TableL   *amptable;
};

// update flags (shift amount is pfield number)
enum {
   kAmp = 1 << 3,
   kGain = 1 << 5,
   kFiltCF = 1 << 6,
   kPan = 1 << 8,
   kBypass = 1 << 9,
   kParam = 1 << 10
};

