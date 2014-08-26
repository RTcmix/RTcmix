//
//  PVNewFilter.h
//  RTcmixTest
//
//  Created by Douglas Scott on 3/15/14.
//
//

#ifndef _PV_NEWFILTER_H_
#define _PV_NEWFILTER_H_

#include "PVFilter.h"

class PVNewFilter : public PVFilter {
public:
	PVNewFilter();
	static PVFilter *	create();
	virtual int			run(float *pvdata, int nvals);
	virtual int			init(double *pp, int args);
protected:
	virtual ~PVNewFilter();
private:
	double *gains;
	int numgains;
};

#endif /* _PV_NEWFILTER_H_ */
