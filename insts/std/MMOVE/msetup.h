/* msetup.h */

#ifndef _MSETUP_H_
#define _MSETUP_H_

#ifdef __cplusplus
extern "C" {
#endif
int get_setup_params(double dimensions[], float *rvbTime, float *absorb,
					 int *useMikes, double *micAngle, double *micPat);
int get_rvb_setup_params(double dimensions[], double matrix[12][12], 
						 float *rvbTime);
#ifdef __cplusplus
}
#endif

#endif	// _MSETUP_H_

