class REVMIX : public Instrument {
   int     inchan, skip, branch, nargs;
   float   amp, pctleft;
   float   *in, *amparray, amptabs[2];

public:
   REVMIX();
   virtual ~REVMIX();
   virtual int init(double *, int);
   virtual int configure();
   virtual int run();
};

