class PAN : public Instrument {
   int     inchan, skip, use_constant_power;
   float   amp;
   float   *in, *amparray, amptabs[2], *panarray, pantabs[2];

public:
   PAN();
   virtual ~PAN();
   int init(float *, int);
   int run();
};

