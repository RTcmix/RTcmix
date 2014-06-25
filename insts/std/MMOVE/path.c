#include <stdio.h>
#include <ugens.h>
#include <common.h>

/* This routine loads the t, x, y, triplets into the two arrays used by
   table() to update sound source location during main loop of move    */

double rholoc[ARRAYSIZE], thetaloc[ARRAYSIZE];
int cartflag;
double mindiff;

double
path (float p[], int n_args)		/* for polar coordinates */
{
    float rhos[100], thetas[100];
    int i, t;
    static double z = 0.017453292;    /* Pi/180 */

    /* check for proper input triplets */

    if (n_args % 3)
       die("path", "Incorrect number of args. Check triplets!");

    /* Separate coordinates */

    t = 0;

    for (i = 0; i < n_args; i+=3)
    {
       rhos[t] = thetas[t] = p[i];	/* the time values */
       rhos[t+1] = p[i+1];
       thetas[t+1] = z * p[i+2]; 	/* convert to radians here */
       t += 2;
    }
    /* Load into arrays */

    setline(rhos,t,ARRAYSIZE,rholoc);
    setline(thetas,t,ARRAYSIZE,thetaloc);
    cartflag = 0;

    return 0.0;
}

double
cpath (float p[], int n_args) 	/* for cartesian coordinates */
{
    float rhos[100], thetas[100];
    int i, t;

    /* check for proper input triplets */

    if (n_args % 3)
       die("cpath", "Incorrect number of args. Check triplets!");

    /* Separate coordinates */

    t = 0;

    for (i = 0; i < n_args; i+=3)
    {
       rhos[t] = thetas[t] = p[i];
       rhos[t+1] = p[i+1];
       thetas[t+1] = p[i+2];
       t += 2;
    }
    /* Load into arrays */

    setline(rhos,t,ARRAYSIZE,rholoc);
    setline(thetas,t,ARRAYSIZE,thetaloc);
    cartflag = 1;

    return 0.0;
}

double
param (float p[], int n_args)	/* parametric setup for polar coordinates */
{
    	int i;
    	double *fun1, *fun2;

    	if (n_args != 2)
       die("param", "Incorrect number of args. Should have 2.");

    	fun1 = floc((int) p[0]);
    	if (fun1 == NULL)
       		die("param", "You haven't made function table %d.", (int) p[0]);
	fun2 = floc((int) p[1]);
    	if (fun2 == NULL)
       		die("param", "You haven't made function table %d.", (int) p[1]);

	for (i = 0; i < ARRAYSIZE; i++)
    	{
       		rholoc[i] = *fun1++;
       		thetaloc[i] = *fun2++;
    	}
	rtcmix_advise("param", "Functions loaded.");
	cartflag = 0;

	return 0.0;
}

double
cparam (float p[], int n_args) /* parametric setup for cartesian coordinates */
{
    	int i;
    	double *fun1, *fun2;

    	if (n_args != 2)
       die("cparam", "Incorrect number of args. Should have 2.");

    	fun1 = floc((int) p[0]);
    	if (fun1 == NULL)
       		die("cparam", "You haven't made function table %d.", (int) p[0]);
	fun2 = floc((int) p[1]);
    	if (fun2 == NULL)
       		die("cparam", "You haven't made function table %d.", (int) p[1]);

	for (i = 0; i < ARRAYSIZE; i++)
    	{
       		rholoc[i] = *fun1++;
       		thetaloc[i] = *fun2++;
    	}
	rtcmix_advise("cparam", "Functions loaded.");
	cartflag = 1;

	return 0.0;
}

double
threshold(float p[], int n_args)
{
	mindiff = p[0];
//	rtcmix_advise("threshold", "Source location updated every %.2f feet.", mindiff);
	rtcmix_advise("threshold", "Source location updated every %.2f msec.",
		   mindiff*1000.0);

	return 0.0;
}

/* this is called by MOVE::localInit() to load global params into instrument */

int get_path_params(double *rhos, double *thetas, int *cartesian, double *mdiff)
{
    int i;
	for(i = 0; i < ARRAYSIZE; i++) {
		rhos[i] = rholoc[i];
		thetas[i] = thetaloc[i];
	}
	*cartesian = cartflag;
	*mdiff = mindiff;
	return 0;
}

// This is used by DMOVE to retrieve params.

int get_params(int *cartesian, double *mdiff)
{
	*cartesian = cartflag;
	*mdiff = mindiff;
	return 0;
}
