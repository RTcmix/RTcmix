#define NTAPS 10

class SROOM : public Instrument {
   int    inchan, insamps, skip, branch;
   int    deltabs[3];
   float  ovamp, aamp;
   float  del[NTAPS], amp[NTAPS];
   float  *in, *delayline, *rvbarrayl, *rvbarrayr, amptabs[2];
   double *amparray;

public:
   SROOM();
   virtual ~SROOM();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
private:
   float distndelset(float, float, float, float, float, float);
};

