typedef struct {
   int    index;
   float  left_amp;
   float  right_amp;
   float  overall_amp;
} Voice;

class JCHOR : public Instrument {
   int     grainsamps, nvoices, skip;
   float   minamp, ampdiff, minwait, waitdiff, seed;
   float   *grain, *amparray, amptabs[2];
   Voice   *voices;

public:
   JCHOR();
   virtual ~JCHOR();
   int init(float *, short);
   int run();
private:
   int grain_input_and_transpose(int, float, float, int, float);
};

