class MYINST : public Instrument {
   int     inchan, skip, branch;
   float   amp, aamp, pctleft;
   float   *in, *amparray, amptabs[2];

public:
   MYINST();
   virtual ~MYINST();
   virtual int init(float *, int);
   virtual int configure();
   virtual int run();
};

