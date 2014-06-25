/* AllPass Interpolating Delay Line Object by Perry R. Cook 1995-96
   This one uses a delay line of maximum length specified on creation,
   and interpolates fractional length using an all-pass filter.  This
   version is more efficient for computing static length delay lines
   (alpha and coeff are computed only when the length is set, there
   probably is a more efficient computational form if alpha is changed
   often (each sample)).
*/
#if !defined(__DLineA_h)
#define __DLineA_h

#include "JGFilter.h"

class DLineA : public JGFilter
{
  protected:  
    long inPoint;
    long outPoint;
    long length;
    double alpha;
    double coeff;
    double lastIn;
  public:
    DLineA(long max_length);
    ~DLineA();
    void clear();
    void setDelay(double length);
    double tick(double sample);
};

#endif
