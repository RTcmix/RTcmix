// setup.h

#ifndef _PV_SETUP_H_
#define _PV_SETUP_H_
#ifdef __cplusplus
#include "PVFilter.h"
int RegisterFilter(FilterCreateFunction *fn, int filterIndex);
int GetFilter(PVFilter **ppFilter);
#endif
#endif	/* _PV_SETUP_H_ */
