#include "FOLLOWER_BASE.h"

class FOLLOWER : public FOLLOWER_BASE {
public:
   FOLLOWER();
   virtual ~FOLLOWER();
protected:
   virtual int pre_init(float *, int);
   virtual int post_init(float *, int);
   virtual float process_sample(float, float);
   virtual void update_params(void);
   virtual const char *instname(void) { return "FOLLOWER"; }
};

