typedef struct {
  float re;
  float im;
  } complex;

#define cmplx( real_part , imaginary_part , x)\
  x.re = (float)real_part;\
  x.im = (float)imaginary_part;

#define add( x , y , z )\
  z.re = x.re + y.re;\
  z.im = x.im + y.im;

#define subtract( x , y , z )\
  z.re = x.re - y.re;\
  z.im = x.im - y.im;

#define multiply( x , y , z )\
  z.re = x.re * y.re - x.im * y.im;\
  z.im = x.re * y.im + x.im * y.re;

#define conjugate( x , z )\
  z.re = x.re;\
  z.im = -x.im;

#define absolute( x , z )\
  conjugate ( x , z ); \
  multiply( x , z , z );\
  z=sqrt(z.re);

#define prtcmplx(x)\
  printf("%f %f \n",x.re,x.im); 

#ifndef sgi
 #ifndef cabs     /* lib/hplset.c needs this */
 #define cabs(x) \
   sqrt((x).re * (x).re + (x).im * (x).im);
 #endif
#endif

