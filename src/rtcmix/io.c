#include "globals.h"
#include <stdio.h>
#include <ugens.h>

extern double m_open(double *, int);

double
m_input(double *p,int n_args)
{
	p[1] = (n_args > 1) ? p[1] : 0.;
	p[2] = 0;
	n_args = 3;
	rtcmix_advise(NULL,"Opening input file as unit %d\n",(int)p[1]);
	return m_open(p,n_args);
}

double
m_output(double *p,int n_args)
{
	int i;
	p[1] = (n_args > 1) ? p[1] : 1.;
	p[2] = 2;
	n_args = 3;
	i = p[0];
	rtcmix_advise("output", "Opening output file as unit %d\n",(int)p[1]);
  	return m_open(p,n_args);
}
