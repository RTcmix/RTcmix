#include <math.h>
double shift(thetao,phio,sr)
float thetao,phio,sr;
{
#define  ABS(x) ((x < 0) ? (-x) : (x))
      float a,b,c,d1,d2,x,dval,theta,phi;
      float pi=3.1415927;
      theta=thetao*pi/(sr/2.);
      phi=phio*pi/(sr/2.);
      a=cos(theta)*tan(phi)+sin(theta);
      b=2.*tan(phi);
      c=cos(theta)*tan(phi)-sin(theta);
/*    d1=(-b+sqrt(b**2-4.*a*c))/(2.*a)
//    d2=(-b-sqrt(b**2-4.*a*c))/(2.*a) */
      x = pow(b,2.) - 4. * a * c;
      d1 = (-b + pow(x,.5))/(2.*a);
      d2 = (-b - pow(x,.5))/(2.*a);
      if(ABS(d1) < 1.) dval=-d1;
      if(ABS(d2) < 1.) dval=-d2;
      return((double)dval);
}
