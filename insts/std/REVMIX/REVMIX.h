class REVMIX : public Instrument {
   int     inchan, skip, branch;
   float   amp, pctleft;
   float   *in, *amparray, amptabs[2];

public:
   REVMIX();
   virtual ~REVMIX();
   int init(float *, int);
   int run();
};

