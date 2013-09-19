// PVRandBend.h -- PVOC data filter which bends each bin randomly

#ifndef _PV_RANDBEND_H_
#define _PV_RANDBEND_H_

#include "PVFilter.h"

class PVRandBend : public PVFilter {
public:
	PVRandBend();
	static PVFilter *	create();
	virtual int			run(float *pvdata, int nvals);
	virtual int			init(double *pp, int args);
protected:
	virtual ~PVRandBend();
private:
	double _pitchRange;
	struct Bin {
		double val;
		double newval;
		double incr;
	} *_bins;
};

#endif /* _PV_RANDBEND_H_ */
