#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>
#include <assert.h>
#include <objlib.h>

#ifdef DEBUG
   #define DPRINT(msg)                    printf((msg))
   #define DPRINT1(msg, arg)              printf((msg), (arg))
   #define DPRINT2(msg, arg1, arg2)       printf((msg), (arg1), (arg2))
   #define DPRINT3(msg, arg1, arg2, arg3) printf((msg), (arg1), (arg2), (arg3))
#else
   #define DPRINT(msg)
   #define DPRINT1(msg, arg)
   #define DPRINT2(msg, arg1, arg2)
   #define DPRINT3(msg, arg1, arg2, arg3)
#endif

class FOLLOWER_BASE : public Instrument {

private:
   int      branch;
   float    *in, caramp, rawmodamp, modamp, smoothness;
   TableL   *amp_table;
   RMS      *gauge;
   JGOnePole  *smoother;
protected:
   int      nargs;
   float    dur, pctleft;

public:
   FOLLOWER_BASE();
   virtual ~FOLLOWER_BASE();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
protected:
   // These are methods a derived class should implement.
   virtual int pre_init(double p[], int n_args) = 0;
   virtual int post_init(double p[], int n_args) = 0;
   virtual float process_sample(float, float) = 0;
   virtual void update_params(double p[]) = 0;
   virtual const char *instname() = 0;
private:
   void update_smoother(double p[]);
};

