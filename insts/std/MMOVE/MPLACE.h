// MPLACE.h

#ifndef _MPLACE_H_
#define _MPLACE_H_

#include "MBASE.h"

class PLACE : public MBASE {
public:
    PLACE();
    virtual ~PLACE();
	virtual int configure();
protected:
    virtual int localInit(double *, int);
    virtual int finishInit(double *);
    virtual int updatePosition(int);
    virtual void get_tap(int, int, int, int);
};

#endif	// _MPLACE_H_
