#ifndef _SFLCFUNCS_H_
#define _SFLCFUNCS_H_

#ifdef __cplusplus
extern "C" {
#endif
	void mdelset(float, float *, int *, int);
	void mdelput(float, float *, int *);
	float mdelget(float *, int, int *);
	float mdliget(float *, float, int *);
#ifdef __cplusplus
};
#endif

#endif // _SFLCFUNCS_H_
