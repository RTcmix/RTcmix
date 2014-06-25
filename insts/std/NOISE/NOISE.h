class NOISE : public Instrument {
   int     inchan, skip, branch, nargs;
   float   amp, pctleft, amptabs[2];
   double  *amparray;

public:
   NOISE();
   virtual ~NOISE();
   virtual int init(double *, int);
   virtual int run();
};

