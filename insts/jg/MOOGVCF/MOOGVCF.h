class MOOGVCF : public Instrument {
   int      inchan, skip, branch;
   float    amp, aamp, pctleft, cf, res;
   float    *in, amptabs[2], cftabs[2], restabs[2];
   double   *amparray, *cfarray, *resarray;
   float    f, p, q, b0, b1, b2, b3, b4;

public:
   MOOGVCF();
   virtual ~MOOGVCF();
   int init(double p[], int n_args);
   int run();
   inline void make_coefficients();
};


/* Calculate filter coefficients f, p and q. */
inline void MOOGVCF :: make_coefficients()
{
   float freq = 2.0 * cf / SR;            /* cf in range [0, 1] */
   q = 1.0 - freq;
   p = freq + (0.8 * freq * q);
   f = p + p - 1.0;
   q = res * (1.0 + 0.5 * q * (1.0 - q + 5.6 * q * q));
}

