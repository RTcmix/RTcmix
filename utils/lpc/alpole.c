#include "../../H/sfheader.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <math.h>


extern int NPOLE;
extern int FRAME;
extern int NPP1;

#define POLEMAX  100
#define FLOAT 4
#define FRAMAX   500 
#define NDATA 4 /* number of data values stored with frame */

alpol(sig, errn, rms1, rms2, c)
double *errn, *rms1, *rms2, *c;
int *sig;
{
	double a[POLEMAX][POLEMAX], v[POLEMAX], b[POLEMAX];
	double x[FRAMAX], y[FRAMAX];
	double *vp=v, *bp=b, *xp=x, *yp=y;
	double sum, sumx, sumy;
	int k1, i, l, k, limit, j;

	for (xp=x; xp-x < FRAME ;++xp,++sig) 
		*xp = *sig;
	k1 = NPOLE + 1;
	for (i=0; i < NPOLE ;++i)  {
		sum = (double) 0.0;
		for (k=NPOLE; k < FRAME ;++k)
			sum += x[k-(i+1)] * x[k];
		v[i] = -sum;
		if (i != NPOLE - 1)  {
			limit = NPOLE - (i+1);
			for (l=0; l < limit ;++l)  {
				sum += x[NPOLE-(i+1)-(l+1)]* x[NPOLE-(l+1)] - x[FRAME-(i+1)-(l+1)]* x[FRAME-(l+1)];
				a[(i+1)+l][l] = a[l][(i+1)+l] = sum;
			}
		}
	}
	sum = (double) 0.0;
	for (k=NPOLE; k < FRAME ;++k)
		sum += pow(x[k], (double) 2.0);
	sumy = sumx = sum;
	for (l=0; l < NPOLE ;++l)  {
		sum += pow(x[NPOLE-(l+1)], (double) 2.0) - pow(x[FRAME-(l+1)], (double) 2.0);
		a[l][l] = sum;
	}
	gauss(a, v, b);
/*	filtn(x, y, b);   */
	for (i=0; i < NPOLE ;++i)
		sumy = sumy - b[i]*v[i];
	*rms1 = sqrt(sumx/(double) (FRAME - k1 + 1) );
	*rms2 = sqrt(sumy/(double) (FRAME - k1 + 1) );
	*errn = pow(((*rms2)/(*rms1)), (double) 2.0);
	for (bp=b; bp-b < NPOLE ;++bp,++c)
		*c = *bp;
	return(0);
}

gauss(aold, bold, b)
double aold[POLEMAX][POLEMAX], *bold, *b;
{
	double amax, dum, pivot;
	double c[POLEMAX], a[POLEMAX][POLEMAX];
	int i, j, k, l, istar, ii, lp;

	istar = 0;
	/* aold and bold untouched by this subroutine */
	for (i=0; i < NPOLE ;++i)  {
		c[i] = bold[i];
		for (j=0; j < NPOLE ;++j)
			a[i][j] = aold[i][j];
	}
	/* eliminate i-th unknown */
	for (i=0; i < NPOLE - 1 ;++i)  {        /* find largest pivot */
		amax = 0.0;
		for (ii=i; ii < NPOLE ;++ii)  {
			if (fabs(a[ii][i]) >= amax)  {
				istar = ii;
				amax = fabs(a[ii][i]);
			}
		}
		if (amax < 1e-20)  {
	       /*	fprintf(stderr, "gauss: ill-conditioned.\n");*/
			return(0);  /* ill-conditioned */
		}
		for (j=0; j < NPOLE ;++j)  {    /* switch rows */
			dum = a[istar][j];
			a[istar][j] = a[i][j];
			a[i][j] = dum;
		}
		dum = c[istar];
		c[istar] = c[i];
		c[i] = dum;
		/* pivot */
		for (j=i+1; j < NPOLE ;++j)  {
			pivot = a[j][i] / a[i][i];
			c[j] = c[j] - pivot * c[i];
			for (k=0; k < NPOLE ;++k)
				a[j][k] = a[j][k] - pivot * a[i][k];
		}
	}       /* return if last pivot is too small */
	if (fabs(a[NPOLE-1][NPOLE-1]) < 1e-20)  {
	/*	fprintf(stderr, "gauss: ill-conditioned.\n");*/
		return(0);      /* ill-conditioned */
	}
	*(b+NPOLE-1) = c[NPOLE-1] / a[NPOLE-1][NPOLE-1];
	/* back substitute */
	for (k=0; k<NPOLE-1; ++k)  {
		l = NPOLE-1 -(k+1);
		*(b+l) = c[l];
		lp = l + 1;
		for (j = lp; j<NPOLE; ++j)
			*(b+l) += -a[l][j] * *(b+j);
		*(b+l) /= a[l][l];
	}
	return(1);
}


filtn(x, y, b)
double x[], b[], y[];
{
	double sum;
	int i, j;
	double *yp;

	for (yp=y; yp-y < NPOLE ;++yp)
		*yp = (double) 0.0;
	yp = y;
	for (i=NPOLE; i < FRAME; ++i)  {
		sum = x[i];
		for (j=0; j < NPOLE ;++j)  {
			sum = sum + b[j] * x[i-(j+1)];
		}
		*(yp+i) = sum;
	}
	return(0);
}
