class MYSYNTH : public Instrument {
   int     inchan, skip, branch;
   float   amp, aamp, pctleft;
   float   *amparray, amptabs[2];

public:
   MYSYNTH();
   virtual ~MYSYNTH();
   int init(double *, int);
   int run();
};

