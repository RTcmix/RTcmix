class REVERBIT : public Instrument {
   int     insamps, skip, dcblock;
   int     deltabs[2];
   float   amp, reverbtime, rtchan_delaytime, reverbpct, cutoff;
   float   prev_in[2], prev_out[2];
   float   *delarray, *rvbarray, *amparray, amptabs[2], *in;
   double  tonedata[3];

public:
   REVERBIT();
   virtual ~REVERBIT();
   int init(float *, short);
   int run();
};

