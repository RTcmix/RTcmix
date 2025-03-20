/* msetup.h */

#ifndef _MSETUP_H_
#define _MSETUP_H_

struct AttenuationParams {
	double minDistance;			// minimum distance for source
	double maxDistance;			// maximum distance for source
	double distanceExponent;	// 1/d^^n factor (n)
};

int get_setup_params(double dimensions[], AttenuationParams *params,
                     double *rvbTime, double *absorb,
					 int *useMikes, double *micAngle, double *micPat);
int get_rvb_setup_params(double dimensions[], double matrix[12][12], 
						 double *rvbTime);

void increment_users();
void decrement_users();
int check_users();

extern "C" {
double mm_space(double p[], int);
double m_mikes(double p[], int);
double m_mikes_off(double p[], int);
double m_oldmatrix(double p[], int);
double m_matrix(double p[], int);
double m_set_attenuation_params(double p[], int);
}

#endif	// _MSETUP_H_

