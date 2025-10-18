#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ugens.h>
#include <math.h>
  
#define DEBUG

double m_sin(double p[], int n_args)
{
	double val;
	val = sin(p[0]);
	return(val);
}

double m_cos(double p[], int n_args)
{
	double val;
	val = cos(p[0]);
	return(val);
}

double m_lowrand(double p[], int n_args)
{
	double v1 = drand48();	
	double v2 = drand48();
	return (v1<v2) ? v1 : v2;
}

double m_highrand(double p[], int n_args)
{
	double v1 = drand48();	
	double v2 = drand48();
	return (v1>v2) ? v1 : v2;
}

double m_trirand(double p[], int n_args)
{
	return 0.5*(drand48()+drand48());
}

double m_constrain(double p[], int n_args)
{
	double lowval = (p[1]<p[2]) ? p[1] : p[2];
	double highval = (p[1]>p[2]) ? p[1] : p[2];
	double bottom = (p[0]<lowval) ? lowval : p[0];
	return (bottom>highval) ? highval : bottom;
}

double m_remap(double p[], int n_args)
{
	double result;
	if (n_args>3)
	{
		double normVal = (p[0]-p[1])/(p[2]-p[1]);
		result = p[3] + normVal*(p[4]-p[3]);
	}
	else
	{
		result = p[1] + p[0]*(p[2]-p[1]);
	}
	return result;
}

double m_gaussrand (double p[], int n_args)
// Normal (Gaussian) distribution
// Code is derived from the GNU Scientific Library,
// src/randist/gauss.c
// values are constrained at the extremes, but the
// effect should be minimal--only 2 or 3 values
// out of 10 million will be affected
{
	double x, y, r2, val;
	do
	{	
		do
		{
			x = (drand48()*2)-1;
			y= (drand48()*2)-1;
			r2 = x * x + y * y;
		} while ((r2 > 1.0) || (r2==0));
		val = y * sqrt(-2.0 * log(r2) /r2);
	} while (val < -5 || val > 5);
	return .5 + val*0.1;
}

double m_prob (double p[], int n_args)
{
	const double mid = (n_args==4) ? p[1] : p[0];
	double num = 0.0;
	double sign;
	// Mara Helmuth's 4-argument prob
	// prob(low,mid,high,tightness)
	if (n_args==4)
	{
		const double low = p[0];
		const double high = p[2];
		const double hirange = high - mid;
		const double lorange = mid - low;
		const double range = (hirange > lorange) ? hirange : lorange;
		const double tight = p[3];
		do {
			num = drand48();       // num is [0,1]
			sign = (num > 0.5) ? 1.0 : -1.0;
			num = mid + (sign * (pow (drand48(), tight) * range));
		} while (num < low || num > high);
	}
	else
	{
		// Joel Matthys's simplified 2-argument prob
		// prob(mid,weight) , both in the range 0-1
		if ((p[1]>0) && (p[1]<1))
		{
			const double tight = log(0.5)/log(p[1]);
			double range = (mid > 0.5) ? mid : 1.0 - mid;
			do {
				num = drand48();       // num is [0,1]
				double sign = (num > 0.5) ? 1.0 : -1.0;
				num = mid + (sign * (pow (drand48(), tight) * range));
			} while (num < 0 || num > 1);
		}
		if (p[1]>=1) num = p[0];
	}
	return num;
}

#ifndef EMBEDDED
/* -------------------------------------------------------------- profile --- */
int
profile()
{
   UG_INTRO("sin", m_sin);
   UG_INTRO("cos", m_cos);
   UG_INTRO("lowrand", m_lowrand);
   UG_INTRO("highrand", m_highrand);
   UG_INTRO("trirand", m_trirand);
   UG_INTRO("gaussrand", m_gaussrand);
   UG_INTRO("prob", m_prob);
   UG_INTRO("constrain", m_constrain);
   UG_INTRO("remap", m_remap);
   return 0;
}
#endif
