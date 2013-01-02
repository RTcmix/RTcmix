// setup.C -- Minc routines needed to set up PVOC utilities

#include <ugens.h>
#include <string.h>
#include "PVFilter.h"
#include "setup.h"
#include <stdlib.h>
#include <stdio.h>
#include <DynamicLib.h>

#ifndef SHAREDLIBDIR
#define SHAREDLIBDIR "."
#endif

extern "C" {
int profile();
double set_filter(float *, int, double *);
double init_filter(float *, int, double *);
}

static const int maxFilters = 8;

FilterCreateFunction	g_filterCtors[maxFilters];	// available filter creators
PVFilter *				g_filters[maxFilters];		// available filters
int						g_currentFilterSlot = 0;
int						g_currentFilters = 0;

int RegisterFilter(FilterCreateFunction createFunction)
{
	if (g_currentFilters + 1 >= maxFilters) {
		rterror(0, "RegisterFilter: exceeded max allowed filters (%d)", 
				maxFilters);
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
	PVFilter *filter;
	if ((filter = g_filters[g_currentFilterSlot]) == NULL) {
		if (g_filterCtors[g_currentFilterSlot] != 0) {
			filter = (*g_filterCtors[g_currentFilterSlot])();
			g_filters[g_currentFilterSlot] = filter;
		}
	}
	*ppFilter = filter;
	return 1;
}

double set_filter(float *p, int n_args, double *pp)
{
	size_t numarg = (size_t) pp[0];
	if (numarg < (size_t)g_currentFilters) {
		if (g_filterCtors[numarg] == 0) {
			rterror("set_filter", "Requested filter slot (%d) is empty", 
					(int)numarg);
			return -1;
		}
		rtcmix_advise("set_filter", "Slot %d selected", (int)numarg);
		g_currentFilterSlot = (int)numarg;
	}
	else {
		const char *filtername = DOUBLE_TO_STRING(pp[0]);
		char dsopath[1024];
		if (filtername[0] == '.' || filtername[0] == '/') {
			strncpy(dsopath, filtername, 1023);
			dsopath[1023] = '\0';
		}
		else {
			sprintf(dsopath, "%s/libPV%s.so", SHAREDLIBDIR, filtername);
		}
		DynamicLib dso;
		if (dso.load(dsopath) == 0) {
			typedef int (*REG)(FilterCreateFunction);
			typedef int (*FUN)(REG);
			FUN registerLib = NULL;
			if (dso.loadFunction(&registerLib, "registerLib") == 0) {
				g_currentFilterSlot = (*registerLib)(RegisterFilter);
				rtcmix_advise("set_filter", "Filter dso loaded and placed in slot %d",
						g_currentFilterSlot);
			}
			else {
				rterror("set_filter", "dso function load returned \"%s\"\n",
						dso.error());
				return -1;
			}
		}
		else {
			rterror("set_filter", "dso load returned \"%s\"\n", dso.error());
			return -1;
		}
	}
	return 1;
}

double init_filter(float *p, int n_args, double *pp)
{
	PVFilter *filter = NULL;
	::GetFilter(&filter);
	if (filter) {
			return filter->init(pp, n_args);
	}
	return -1;
}

int profile()
{
	UG_INTRO("set_filter", set_filter);
	UG_INTRO("init_filter", init_filter);
	return 0;
}
