/* One Zero Filter Class, by Perry R. Cook, 1995-96
   The parameter <gain> is an additional gain parameter applied to the
   filter on top of the normalization that takes place automatically.
   So the net max gain through the system equals the value of <gain>.
   <sgain> is the combination of <gain> and the normalization parameter,
   so if you set the zeroCoeff to alpha, sgain is always set to
   gain / (1.0 + fabs(alpha)).
*/

#if !defined(__OneZero_h)
#define __OneZero_h

#include "Filter.h"

class OneZero : public Filter
{
  protected:  
    double zeroCoeff;
    double sgain;
  public:
    OneZero();
    ~OneZero();
    void clear();
    void setGain(double aValue);
    void setCoeff(double aValue);
    double tick(double sample);
};

#endif
