// PVFilterTest.h -- test code for new filters

#ifndef _PV_FILTERTEST_H_
#define _PV_FILTERTEST_H_

#include "PVFilter.h"

class PVFilterTest : public PVFilter {
public:
	PVFilterTest();
	static PVFilter *	create();
	virtual int			run(float *pvdata, int nvals);
	virtual int			init(double *pp, int args);
protected:
	virtual ~PVFilterTest();
private:
	double val1, val2, val3;
};

#endif /* _PV_FILTERTEST_H_ */
