#include "FOLLOWER_BASE.h"

class FOLLOWER : public FOLLOWER_BASE {
public:
   FOLLOWER();
   virtual ~FOLLOWER();
protected:
   virtual int pre_init(double p[], int n_args);
   virtual int post_init(double p[], int n_args);
   virtual float process_sample(float, float);
   virtual void update_params(double p[]);
   virtual const char *instname() { return "FOLLOWER"; }
};

