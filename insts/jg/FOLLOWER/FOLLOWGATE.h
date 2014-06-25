#include "FOLLOWER_BASE.h"

typedef enum {
   belowThreshold,
   aboveThreshold
} PowerState;

class FOLLOWGATE : public FOLLOWER_BASE {
   float    threshold, range, oneoverSR;
   float    attack_time, release_time, attack_rate, release_rate;
   TableL   *thresh_table, *range_table;
   Envelope *envelope;
   PowerState state;
public:
   FOLLOWGATE();
   virtual ~FOLLOWGATE();
protected:
   virtual int pre_init(double p[], int n_args);
   virtual int post_init(double p[], int n_args);
   virtual float process_sample(float, float);
   virtual void update_params(double p[]);
   virtual const char *instname() { return "FOLLOWGATE"; }
};

