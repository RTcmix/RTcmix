// PVFilter.C -- base class for pvoc data filters

#include "PVFilter.h"

PVFilter::PVFilter()
{
}

PVFilter::~PVFilter()
{
}

int
PVFilter::init(double *p, int nargs) { return 1; }
