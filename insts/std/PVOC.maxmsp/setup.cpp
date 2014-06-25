// setup.C -- Minc routines needed to set up PVOC utilities

#include <ugens.h>
#include "PVFilter.h"
#include "setup.h"

extern "C" {
// BGG mm -- changed this for max/msp
//int profile();
double set_filter(float *p, int, double *pp);
}

static const int maxFilters = 8;

FilterCreateFunction	*g_filterCtors[maxFilters];	// available filter creators
int			g_currentFilterSlot = 0;
int			g_currentFilters = 0;

int RegisterFilter(FilterCreateFunction *createFunction)
{
	if (g_currentFilters + 1 >= maxFilters) {
		rterror(0, "RegisterFilter: exceeded max allowed filters (%d)", maxFilters);
		return -1;
	}
	g_filterCtors[g_currentFilters] = createFunction;
	++g_currentFilters;
	return g_currentFilters - 1;	// The value of the slot
}

// The following is called by PVOC::init() to load the filter into the
// PVOC instance

int GetFilter(PVFilter **ppFilter)
{
	PVFilter *filter = 0;
	if (g_filterCtors[g_currentFilterSlot] != 0)
		filter = (*g_filterCtors[g_currentFilterSlot])();
	*ppFilter = filter;
	return 1;
}

double set_filter(float *p, int n_args, double *pp)
{
	int filterSlot = (int) pp[0];
	if (filterSlot > g_currentFilters || g_filterCtors[filterSlot] == 0) {
		rterror("set_filter", "Requested filter slot (%d) is empty", 
				filterSlot);
		return -1;
	}
	rtcmix_advise("set_filter", "Slot %d selected", filterSlot);
	g_currentFilterSlot = filterSlot;
	return 1;
}

/* BGG mm -- consolidates in src/rtcmix/rtprofile.cpp
int profile()
{
	UG_INTRO("set_filter", set_filter);
	return 0;
}
*/
