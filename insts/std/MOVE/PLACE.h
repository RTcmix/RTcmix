#include "BASE.h"

#define NPRIMES   5000
#define NCOEFFS   512
#define BUFLEN	  256
#define MAX_INPUTS  4

class PLACE : public BASE {
public:
    PLACE();
    virtual ~PLACE();
	virtual int configure();
protected:
    virtual int localInit(float *, int);
    virtual int finishInit(double, double *);
    virtual int updatePosition(int);
    virtual void get_tap(int, int, int, int);
};

