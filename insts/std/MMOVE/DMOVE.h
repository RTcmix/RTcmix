/* DMOVE.h -- Dynamic multi-call version of MOVE */

#ifndef _DMOVE_H_
#define _DMOVE_H_

#include "MBASE.h"

class DMOVE : public MBASE {
public:
    DMOVE();
    virtual ~DMOVE();
protected:
    virtual int localInit(double *, int);
    virtual int finishInit(double, double *);
    virtual int updatePosition(int);
    virtual void get_tap(int, int, int, int);
private:
    double *rholoc, *thetaloc;
    double R_old, T_old, mindiff;
    int m_updateSamps, m_updateCount;
    double oldOutlocs[2][13];
};

#endif	// _DMOVE_H_
