typedef struct {
   int    index;
   float  left_amp;
   float  right_amp;
   float  overall_amp;
} Voice;

class JCHOR : public Instrument {
   int     grainsamps, nvoices, skip, maintain_indur, grain_done, inchan;
   int     winarraylen;
   float   inskip, indur, transpose, minamp, ampdiff, minwait, waitdiff, seed;
   float   amptabs[2], wintabs[2];
   double  *amparray, *winarray;
   float   *grain, *in;
   Voice   *voices;

public:
   JCHOR();
   virtual ~JCHOR();
   int init(double p[], int n_args);
   int run();
private:
   int setup_voices();
   int grain_input_and_transpose();
};

