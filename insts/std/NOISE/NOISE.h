class NOISE : public Instrument {
   int     inchan, skip, branch, nargs;
   float   amp, pctleft;
   float   *amparray, amptabs[2];

public:
   NOISE();
   virtual ~NOISE();
   virtual int init(double *, int);
   virtual int run();
};

