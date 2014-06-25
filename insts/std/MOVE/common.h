// common.h -- decls for common free functions

#include <math.h>

#define MAXTERMS  33       // also defined in firdata.h
#define NANGLES   24
#define ARRAYSIZE 4096		// used for Rho, Theta arrays

#ifdef __cplusplus

extern "C" {
#include <ugens.h>
}

const double MACH1 = 1080.0;

// inline functions

/* ----------------------------------------------------------------- tone --- */
/* tone is a simple 1st order recursive lowpass filter
*/

inline double
tone(double sig, double data[3])
{
    double out = data[0] * sig + data[1] * data[2];
    return data[2] = out;
}

#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif

/* common non-inline functions */

extern long MFP_samps(float, double []);
extern int close_prime(int, int, int []);
extern void setup_trigfuns(void);
extern float cycle(float, double, int);
extern void binaural(double, double, double, double, double,
                                                          double *, double *);
extern void fir(double *, long, int, double *, double *, int);
extern void setfir(double, int, int, double *, double *);
extern void scale(double *, int, double);
extern void btone(double *, int, double data[3]);
extern void air(double *, int, double[3]);
extern void wall(double *, int, double[3]);
extern void check_denormals(double *, int);
extern void copyBuf(double *to, double *from, int len);
extern void addBuf(double *to, double *from, int len);
extern void copyScaleBuf(double *to, double *from, int len, double gain);
extern void addScaleBuf(double *to, double *from, int len, double gain);
extern double wrap(double);

// Versions which write to floating point output buffers
extern void addScaleBufToOut(float *to, double *from, int len, int toChannels, double gain);
extern void copyBufToOut(float *to, double *from, int len, int toChannels);

// global constants


#ifdef __cplusplus
}
#endif
