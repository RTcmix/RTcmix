/* DMOVE.h -- Dynamic multi-call version of MOVE */

#ifndef _DMOVE_H_
#define _DMOVE_H_

#include "MSTEREOBASE.h"
#include "MOVEBASE.h"

class DMOVE : public MSTEREOBASE, public MOVEBASE<2> {
public:
    DMOVE();
    virtual ~DMOVE();
protected:
    virtual int localInit(double *p, int n_args);
    virtual int finishInit(double *ringdur);
    virtual int updatePosition(int);
    virtual void get_tap(int, int, int, int);
private:
    double *rholoc, *thetaloc;
    double R_old, T_old, mindiff;
    int m_updateSamps, m_updateCount;
    double oldOutlocs[2][13];
};

#endif	// _DMOVE_H_
