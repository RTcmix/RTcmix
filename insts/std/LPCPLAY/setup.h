// setup.h -- decls for Minc utilities

#ifndef _SETUP_H_
#define _SETUP_H_

extern "C" {
int profile();
double dataset(double p[], int n_args);
double lpcstuff(double *p, int n_args);
double freset(double *p, int n_args);
double setdev(double *p, int n_args);
double setdevfactor(double *p, int n_args);
double set_thresh(double *p, int n_args);
double set_hnfactor(double *p, int n_args);
double use_autocorrect(double *p, int n_args);
double use_fix_pitch_octaves(double *p, int n_args);
double use_fix_pitch_gaps(double *p, int n_args);
double use_pitch_smoothing(double *p, int n_args);
}

#endif	//	 _SETUP_H_
