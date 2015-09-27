// PVTransBend.h -- PVOC data filter which bends each bin randomly

#ifndef _PV_RANDBEND_H_
#define _PV_RANDBEND_H_

#include "PVFilter.h"

class PVTransBend : public PVFilter {
public:
	PVTransBend();
	static PVFilter *	create();
	virtual int			run(float *pvdata, int nvals);
	virtual int			init(double *pp, int args);
protected:
	virtual ~PVTransBend();
private:
	double *	_exptable;
	int			_totalCalls;
	int			_currentCall;
};

#endif /* _PV_RANDBEND_H_ */
