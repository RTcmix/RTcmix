#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define FORWARD 1
#define INVERSE 0
#define CABS(x) hypot( (x).re, (x).im )

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { float re; float im; } complex;

complex cadd(), csub(), cmult(), smult(), cdiv(), conjg(), csqrt();

extern const complex zero;
extern const complex one;
extern float TWOPI;

/* prototypes */
void findroots(complex a[], complex r[], int M);
complex scmult(float s, complex x);
void makewindows(float H[], float A[], float S[], int Nw, int N, int I, int osc);
void rfft(float x[], int N, int forward);
void cfft(float x[], int NC, int forward);
void fold(float I[], float W[], int Nw, float O[], int N, int n);
void overlapadd(float I[], int N, float W[], float O[], int Nw, int n);
float lpa(float x[], int N, float b[], int M);
float lpamp(float omega, float a0, float *coef, int M);

#ifdef __cplusplus
}
#endif
