class NOISE : public Instrument {
   int     inchan, skip, branch;
   float   amp, aamp, pctleft;
   float   *amparray, amptabs[2];

public:
   NOISE();
   virtual ~NOISE();
   int init(double *, int);
   int run();
};

