class MYSYNTH : public Instrument {
   int     inchan, skip, branch;
   float   amp, pctleft;
   float   *amparray, amptabs[2];

public:
   MYSYNTH();
   virtual ~MYSYNTH();
   int init(float *, int);
   int run();
};

