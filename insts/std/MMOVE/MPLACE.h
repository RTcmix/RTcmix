// MPLACE.h

#ifndef _MPLACE_H_
#define _MPLACE_H_

#include "MSTEREOBASE.h"

class MPLACE : public MSTEREOBASE {
public:
    MPLACE();
    virtual ~MPLACE();
	virtual int configure();
protected:
    virtual int localInit(double *, int);
    virtual int finishInit(double *);
    virtual int updatePosition(int);
    virtual void get_tap(int, int, int, int);
};

#endif	// _MPLACE_H_
