#include "FOLLOWER_BASE.h"

typedef enum {
   belowThreshold,
   aboveThreshold
} PowerState;

class FOLLOWGATE : public FOLLOWER_BASE {
   float    threshold, range, attack_rate, release_rate;
   TableL   *thresh_table, *range_table;
   Envelope *envelope;
   PowerState state;
public:
   FOLLOWGATE();
   virtual ~FOLLOWGATE();
protected:
   virtual int pre_init(float *, int);
   virtual int post_init(float *, int);
   virtual float process_sample(float, float);
   virtual void update_params(void);
   virtual const char *instname(void) { return "FOLLOWGATE"; }
};

