#include "speakers.h"

class NPAN : public Instrument {
   enum { PolarMode = 0, CartesianMode = 1 };

   int     inchan, mode, skip, branch, num_speakers;
   double  amp, prev_angle, src_angle, src_distance, min_distance, src_x, src_y;
   Speaker *speakers[MAX_SPEAKERS];
   float   *in;

   int usage();
   int getmode();
   void setgains();
   void doupdate();
   void dumpspeakers();

public:
   NPAN();
   virtual ~NPAN();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

