/* Linearly Interpolating Delay Line Object by Perry R. Cook 1995-96
   This one uses a delay line of maximum length specified on creation,
   and linearly interpolates fractional length. It is designed to be
   more efficient if the delay length is not changed very often.

   JGG added alternative API, to be used in place of setDelay / tick.
   Useful mainly for situations where you want multiple taps, which you
   can't do with the other API. To summarize the two API's:

    (1) setDelay / tick:
          setDelay(lag);
          output = tick(input);     // output is <lag> samps older than input

    (2) putSample / getSample:
          putSample(input);         // like tick, but no output returned
          output = getSample(lag);  // output is lag samps older than last input
          out2 = getSample(lag2);   // can call again before next putSample

   The <lag> argument for setDelay and getSample can have a fractional
   part, in which case linear interpolation determines the output from
   the delay line.

   Note that in the interest of efficiency, putSample and getSample do
   not maintain the correct values for outPoint and omAlpha. (If you want
   to use the first API after having used the second one for a DLineL
   object, call setDelay to init outPoint and omAlpha.)
*/
#if !defined(__DLineL_h)
#define __DLineL_h

#include "JGFilter.h"

class DLineL : public JGFilter
{
  protected:  
    long inPoint;
    long outPoint;
    long length;
    double alpha;
    double omAlpha;
  public:
    DLineL(long max_length);  
    ~DLineL();  
    void clear();
    void setDelay(double lag);
    double tick(double input);
    void putSample(double input);
    double getSample(double lag);
};

#endif
