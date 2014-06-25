class PAN : public Instrument {
   int     nargs, inchan, skip, branch;
   float   amp, prevpan, pan[2];
   float   *in, amptabs[2], pantabs[2];
   double  *amparray, *panarray;

   void doupdate();
public:
   PAN();
   virtual ~PAN();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

// update flags (shift amount is pfield index)
enum {
	kAmp = 1 << 3,
	kUseConstPower = 1 << 5,
	kPan = 1 << 6
};

