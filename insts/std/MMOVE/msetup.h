/* msetup.h */

#ifndef _MSETUP_H_
#define _MSETUP_H_

struct AttenuationParams {
	double minDistance;			// minimum distance for source
	double maxDistance;			// maximum distance for source
	double distanceExponent;	// 1/d^^n factor (n)
};

int get_setup_params(double dimensions[], AttenuationParams *params,
					 float *rvbTime, float *absorb,
					 int *useMikes, double *micAngle, double *micPat);
int get_rvb_setup_params(double dimensions[], double matrix[12][12], 
						 float *rvbTime);

extern "C" {
double space(float p[], int);
double mikes(float p[], int);
double mikes_off(float p[], int);
double oldmatrix(float p[], int);
double matrix(float p[], int);
double set_attenuation_params(float p[], int);
}

#endif	// _MSETUP_H_

