class REVERBIT : public Instrument {
   bool    dcblock, usefilt;
   int     insamps, branch;
   int     deltabs[3];
   float   amp, reverbtime, rtchan_delaytime, reverbpct, cutoff;
   float   prev_in[2], prev_out[2];
   float   *in, *delarray, *rvbarray, amptabs[2];
   double  *amparray;
   double  tonedata[3];

   void doupdate();
public:
   REVERBIT();
   virtual ~REVERBIT();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

// update flags (shift amount is pfield index)
enum {
   kRvbTime = 1 << 4,
   kRvbPct = 1 << 5,
   kCutoffFreq = 1 << 7
};

