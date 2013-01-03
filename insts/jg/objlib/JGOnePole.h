/* One Pole Filter Class, by Perry R. Cook, 1995-96
   (setFreq method added by JG)

   The parameter <gain> is an additional gain parameter applied to the
   filter on top of the normalization that takes place automatically.
   So the net max gain through the system equals the value of gain.
   <sgain> is the combination of gain and the normalization parameter,
   so if you set the poleCoeff to alpha, sgain is always set to
   gain * (1.0 - fabs(alpha)).

   (removed output[] -- function already served by lastOutput.  -JGG)
*/
#if !defined(__JGOnePole_h)
#define __JGOnePole_h

#include "JGFilter.h"

class JGOnePole : public JGFilter
{
  protected:  
    double poleCoeff;
    double sgain;
  public:
    JGOnePole(double srate);
    ~JGOnePole();
    void clear();
    void setFreq(double freq);
    void setPole(double aValue);
    void setGain(double aValue);
    double tick(double sample);
};

#endif
