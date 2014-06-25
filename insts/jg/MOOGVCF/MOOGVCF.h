class MOOGVCF : public Instrument {
   bool     bypass;
   int      nargs, inchan, skip, branch;
   float    amp, pctleft, cf, res;
   float    *in, amptabs[2], cftabs[2], restabs[2];
   double   *amparray, *cfarray, *resarray;
   float    f, p, q, b0, b1, b2, b3, b4;

   void doupdate();
   inline void make_coefficients();
public:
   MOOGVCF();
   virtual ~MOOGVCF();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};


// Calculate filter coefficients f, p and q.
inline void MOOGVCF :: make_coefficients()
{
   float freq = 2.0 * cf / SR;            // cf in range [0, 1]
   q = 1.0 - freq;
   p = freq + (0.8 * freq * q);
   f = p + p - 1.0;
   q = res * (1.0 + 0.5 * q * (1.0 - q + 5.6 * q * q));
}

// update flags (shift amount is pfield index)
enum {
	kAmp = 1 << 3,
	kPan = 1 << 5,
	kBypass = 1 << 6,
	kCutoff = 1 << 7,
	kResonance = 1 << 8
};

