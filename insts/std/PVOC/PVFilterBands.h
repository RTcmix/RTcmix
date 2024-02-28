//
//  PVFilterBands.h
//  RTcmix
//
//  Created by Douglas Scott on 9/19/23.
//
//

#ifndef PVFilterBands_h
#define PVFilterBands_h

#include "PVFilter.h"

class PVFilterBands : public PVFilter
{
public:
	static PVFilter *	create();
	virtual int			run(float *pvdata, int nvals);
	virtual int			init(double *pp, int args);
protected:
	PVFilterBands();
	virtual 			~PVFilterBands();
private:
	float *				_binGains;
};
#endif /* defined(PVFilterBands_h) */
