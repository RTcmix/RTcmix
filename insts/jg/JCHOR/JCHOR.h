typedef struct {
   int    index;
   float  left_amp;
   float  right_amp;
   float  overall_amp;
} Voice;

class JCHOR : public Instrument {
   bool    maintain_indur, grain_done;
   int     nargs, grainsamps, nvoices, skip, branch, inchan;
   int     winarraylen;
   float   inskip, indur, transpose, minamp, ampdiff, minwait, waitdiff, seed;
   float   amp, amptabs[2], wintabs[2];
   double  *amparray, *winarray;
   float   *grain, *in;
   Voice   *voices;

   void doupdate();
public:
   JCHOR();
   virtual ~JCHOR();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
private:
   int setup_voices();
   int grain_input_and_transpose();
};

// update flags (shift amount is pfield index)
enum {
	kMinAmp = 1 << 7,
	kMaxAmp = 1 << 8,
	kMinWait = 1 << 9,
	kMaxWait = 1 << 10,
	kAmp = 1 << 13
};

