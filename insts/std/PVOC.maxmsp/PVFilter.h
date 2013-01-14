// PVFilter.h -- encapsulates code to modify the freq,amp pairs used by PVOC

#ifndef _PV_FILTER_H_
#define _PV_FILTER_H_

#include <RefCounted.h>

class PVFilter : public RefCounted {
public:
	PVFilter();
	virtual int		run(float *pvdata, int nvals)=0;
protected:
	virtual ~PVFilter();
};

// Each PVOC filter subclass must define a function which returns a new instance
//	of the subclass, and pass this function to RegisterFilter().

typedef PVFilter * (*FilterCreateFunction)(void);

#endif /* _PV_FILTER_H_ */
