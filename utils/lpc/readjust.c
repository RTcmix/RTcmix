#include "../../H/ugens.h"
#define RESIDAMP     0
#define RMSAMP       1
#define THRESH       2
#define PITCH	     3
#define MAXPOLES 60

float deviation(frame1,frame2,weight,throsh)
float frame1,frame2,throsh,weight;
{
	float c[MAXPOLES+4];
	int i,j;
	float diff,xweight,sum;
	xweight = sum = 0;
	for(j=0,diff=0,i=(int)frame1; i<(int)frame2; i++) {
		getfr((float)i,c);
		if((c[THRESH] <= throsh) || (throsh < 0.)) {
		sum += ((ABS((c[PITCH] - weight))) * c[RMSAMP]);
		xweight +=  c[RMSAMP];
		}
	}
	printf("average deviation = %f\n",sum/xweight);
	return(sum/xweight);
}

adjust(actdev,desdev,actweight,pchval,framefirst,framelast)
float actdev,desdev,actweight,*pchval,framefirst,framelast;
{
	int i,j;
	float x,devfact;
/* two heuristics here: only shrinking range, and no pitches < 50 hz */
/*	devfact = (desdev > actdev) ? 1. : desdev/actdev; */
	devfact =  desdev/actdev;
	for(j=0,i=(int)framefirst; i<=(int)framelast; i++,j++) {
		x = (pchval[j]-actweight) * devfact + actweight;
		pchval[j] = (x >50) ? x : pchval[j];
	}
}

readjust(maxdev,pchval,firstframe,lastframe,thresh,weight)
float maxdev,weight,*pchval,firstframe,lastframe,thresh;
{
	float deviation();
	float dev;
	dev = deviation(firstframe,lastframe,weight,thresh);
	if (!dev) dev=.0001;
	if(maxdev)
	adjust(dev,maxdev,weight,pchval,firstframe,lastframe);
}
