class JDELAY : public Instrument {
   int     insamps, skip, inchan, dcblock, prefadersend;
   int     deltabs[2];
   float   amp, wait, regen, cutoff, percent_wet, spread;
   float   prev_in, prev_out;
   float   *in, *delarray, *amptable, amptabs[2];
   double  tonedata[3];

public:
   JDELAY();
   virtual ~JDELAY();
   int init(double p[], int n_args);
   int run();
};

