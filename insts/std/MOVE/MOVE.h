/* MOVE.h -- Real Time version of 'move'.  Derived from BASE */

#include "BASE.h"

class MOVE : public BASE {
public:
    MOVE();
    virtual ~MOVE();
protected:
    virtual const char * name() { return "MOVE"; }
    virtual int localInit(float *, short);
    virtual int finishInit(double, double *);
    virtual int updatePosition(int);
    virtual void get_tap(int, int, int, int);
private:
    float *rholoc, *thetaloc;
    float tabr[2], tabt[2];
    double R_old, T_old, mindiff;
    double oldOutlocs[2][13];
};
