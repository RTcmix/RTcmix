class PAN : public Instrument {
   int     inchan, skip;
   float   amp;
   float   *amparray, amptabs[2], *panarray, pantabs[2], *in;

public:
   PAN();
   virtual ~PAN();
   int init(float *, short);
   int run();
};

