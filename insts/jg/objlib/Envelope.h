/* Envelope Class, Perry R. Cook, 1995-96
   This is the base class for envelopes. This one is capable of ramping
   state from where it is to a target value by a rate. It also responds
   to simple KeyOn and KeyOff messages, ramping to 1.0 on keyon and to
   0.0 on keyoff. There are two tick (update value) methods, one returns
   the value, and other returns 0 if the envelope is at the target value
   (the state bit).
*/
#if !defined(__Envelope_h)
#define __Envelope_h

#include "objdefs.h"

/* Envelope states (returned by getState and informTick methods) */
#define ENV_HOLDING  0
#define ENV_RAMPING  1

class Envelope
{
  protected:  
    double value;
    double target;
    double rate;
    double _sr;
    int state;
  public:
    Envelope(double srate);
    virtual ~Envelope();
    void keyOn();
    void keyOff();
    void setRate(double aRate);
    void setTime(double aTime);
    void setTarget(double aTarget);
    void setValue(double aValue);
    int getState();
    double tick();
    int informTick();
    double lastOut();
};

#endif
