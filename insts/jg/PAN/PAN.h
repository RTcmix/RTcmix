class PAN : public Instrument {
   int     inchan, skip, use_constant_power;
   float   amp;
   float   *in, amptabs[2], pantabs[2];
   double  *amparray, *panarray;

public:
   PAN();
   virtual ~PAN();
   int init(double p[], int n_args);
   int run();
};

