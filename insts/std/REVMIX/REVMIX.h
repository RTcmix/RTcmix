class REVMIX : public Instrument {
   int     inchan, skip, branch, nargs;
   float   amp, pctleft, *in, amptabs[2];
   double  *amparray;

public:
   REVMIX();
   virtual ~REVMIX();
   virtual int init(double *, int);
   virtual int configure();
   virtual int run();
};

