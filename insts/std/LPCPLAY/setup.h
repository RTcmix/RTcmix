// setup.h -- decls for Minc utilities

#ifndef _SETUP_H_
#define _SETUP_H_

extern "C" {
int profile();
double dataset(float *p, int n_args, double *pp);
double lpcstuff(float *p, int n_args);
double freset(float *p, int n_args);
double setdev(float *p, int n_args);
double setdevfactor(float *p, int n_args);
double set_thresh(float *p, int n_args);
double set_hnfactor(float *p, int n_args);
double use_autocorrect(float *p, int n_args);
}

#endif	//	 _SETUP_H_
