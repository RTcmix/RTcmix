#define NTAPS 10

class SROOM : public Instrument {
   int    inchan, insamps, skip;
   int    deltabs[3];
   float  ovamp;
   float  del[NTAPS], amp[NTAPS];
   float  *in, *delayline, *rvbarrayl, *rvbarrayr, amptabs[2];
   double *amparray;

public:
   SROOM();
   virtual ~SROOM();
   int init(double p[], int n_args);
   int run();
private:
   float distndelset(float, float, float, float, float, float);
};

