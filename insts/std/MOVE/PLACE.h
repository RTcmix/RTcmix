#include "BASE.h"

class PLACE : public BASE {
public:
    PLACE();
    virtual ~PLACE();
	virtual int configure();
protected:
    virtual int localInit(double *, int);
    virtual int finishInit(double, double *);
    virtual int updatePosition(int);
    virtual void get_tap(int, int, int, int);
};

