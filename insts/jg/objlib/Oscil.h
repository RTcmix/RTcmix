/* Oscillator base class, by John Gibson
*/

#if !defined(__Oscil_h)
#define __Oscil_h

#include "objdefs.h"

#define DEFAULT_WAVETABLE_SIZE  1024   // only for auto-generated ones

class Oscil
{
  protected:
    double   *table;             // pointer to wave table
    int      size;               // size of table
    double   phase;              // current index into table
    double   increment;          // increments phase
    MY_FLOAT lastOutput;
  public:
    Oscil();
    virtual ~Oscil();
    void setPhase(MY_FLOAT aPhase);
    MY_FLOAT lastOut();
};

double *getSineTable();

#endif

