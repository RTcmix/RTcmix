#include "BASE.h"

#define NPRIMES   5000
#define NCOEFFS   512
#define BUFLEN	  256
#define MAX_INPUTS  4

class PLACE : public BASE {
public:
    PLACE();
    virtual ~PLACE();
protected:
    virtual const char * name() { return "PLACE"; }
    virtual int localInit(float *, short);
    virtual int finishInit(double, double *);
    virtual int updatePosition(int);
    virtual void get_tap(int, int, int, int);
};

