/* ADSR Subclass of the Envelope Class, by Perry R. Cook, 1995-96
   This is the traditional ADSR (Attack, Decay, Sustain, Release) envelope.
   It responds to simple KeyOn and KeyOff messages, keeping track of it's
   state. There are two tick (update value) methods, one returns the value,
   and other returns the state (0 = A, 1 = D, 2 = S, 3 = R).
*/
#if !defined(__ADSR_h)
#define __ADSR_h

#include "Envelope.h"

/* Envelope states (returned by getState and informTick methods).
   Note that these differ from the states in the Envelope class.
*/
#define ADSR_ATTACK  0
#define ADSR_DECAY   1
#define ADSR_SUSTAIN 2
#define ADSR_RELEASE 3
#define ADSR_END     4

class ADSR : public Envelope
{
  protected:  
    double attackRate;
    double decayRate;
    double sustainLevel;
    double releaseRate;
  public:
    ADSR(double srate);
    ~ADSR();
    void keyOn();
    void keyOff();
    void setAttackRate(double aRate);
    void setDecayRate(double aRate);
    void setSustainLevel(double aLevel);
    void setReleaseRate(double aRate);
    void setAttackTime(double aTime);
    void setDecayTime(double aTime);
    void setReleaseTime(double aTime);
    void setAllTimes(double attTime, double decTime, double susLevel,
                                                            double relTime);
    void setTarget(double aTarget);
    void setValue(double aValue);
    double tick();
    int informTick();  
    double lastOut();
};

#endif
