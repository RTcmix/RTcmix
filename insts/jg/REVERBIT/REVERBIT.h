class REVERBIT : public Instrument {
   int     insamps, skip, dcblock;
   int     deltabs[2];
   float   amp, reverbtime, rtchan_delaytime, reverbpct, cutoff;
   float   prev_in[2], prev_out[2];
   float   *in, *delarray, *rvbarray, *amparray, amptabs[2];
   double  tonedata[3];

public:
   REVERBIT();
   virtual ~REVERBIT();
   int init(double p[], int n_args);
   int run();
};

