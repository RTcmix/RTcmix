#include <Instrument.h>

class QPAN : public Instrument {
   int     inchan, branch;
   double  amp, src_x, src_y, gains[4];
   double  speaker_angles[4];
   float   *in;

   int usage();
   void doupdate();

public:
   QPAN();
   virtual ~QPAN();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

// update flags (shift amount is pfield index)
enum {
   kAmp = 1 << 3,
   kSrcX = 1 << 4,
   kSrcY = 1 << 5
};

