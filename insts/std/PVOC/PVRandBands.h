//
//  PVRandBands.h
//  RTcmix
//
//  Created by Douglas Scott on 9/16/15.
//
//

#ifndef PVRandBands_h
#define PVRandBands_h

#include "PVFilter.h"

class PVRandBands : public PVFilter
{
public:
	static PVFilter *	create();
	virtual int			run(float *pvdata, int nvals);
	virtual int			init(double *pp, int args);
protected:
	PVRandBands();
	virtual 			~PVRandBands();
private:
	float *				_binGains;
};
#endif /* defined(PVRandBands_h) */
