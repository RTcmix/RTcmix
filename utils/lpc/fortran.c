#include <signal.h>
#include <stdio.h>
#include "f2c.h" 

#ifdef KR_headers 
extern void sig_die();  
VOID z_div(c, a, b) 
doublecomplex *a, *b, *c; 
#else 
extern void sig_die(char*, int); 
void z_div(doublecomplex *c, doublecomplex *a, doublecomplex *b) 
#endif 
{
double ratio, den; 
double abr, abi; 

if( (abr = b->r) < 0.) 
        abr = - abr;  if( (abi = b->i) < 0.) 
        abi = - abi;  if( abr <= abi ) 
        {
        if(abi == 0) 
                sig_die("complex division by zero", 1); 
        ratio = b->r / b->i ; 
        den = b->i * (1 + ratio*ratio); 
        c->r = (a->r*ratio + a->i) / den; 
        c->i = (a->i*ratio - a->r) / den; 
        }

else
        {
        ratio = b->i / b->r ; 
        den = b->r * (1 + ratio*ratio); 
        c->r = (a->r + a->i*ratio) / den; 
        c->i = (a->i - a->r*ratio) / den; 
        }

}


#ifdef KR_headers
VOID s_stop(s, n) char *s; ftnlen n;
#else
#undef abs
#undef min
#undef max
#include "stdlib.h"
#ifdef __cplusplus
extern "C" {
#endif

int s_stop(char *s, ftnlen n)
#endif
{
int i;

if(n > 0)
        {
        fprintf(stderr, "STOP ");
        for(i = 0; i<n ; ++i)
                putc(*s++, stderr);
        fprintf(stderr, " statement executed\n");
        }
#ifdef NO_ONEXIT
#endif
exit(0);
#ifdef __cplusplus
return 0; /* NOT REACHED */
}
#endif
}

#ifdef KR_headers
double pow();
double pow_dd(ap, bp) doublereal *ap, *bp;
#else
#undef abs
#include "math.h"
double pow_dd(doublereal *ap, doublereal *bp)
#endif
{
return(pow(*ap, *bp) );
}

#ifndef SIGIOT
#ifdef SIGABRT
#define SIGIOT SIGABRT
#endif
#endif

#ifdef KR_headers
void sig_die(s, kill) register char *s; int kill;
#else
#include "stdlib.h"
#ifdef __cplusplus
extern "C" {
#endif

void sig_die(register char *s, int kill)
#endif
{
	/* print error message, then clear buffers */
	fprintf(stderr, "%s\n", s);

	if(kill)
		{
		fflush(stderr);

		fflush(stderr);
		/* now get a core */
#ifdef SIGIOT
		signal(SIGIOT, SIG_DFL);
#endif
		abort();
		}
	else {
#ifdef NO_ONEXIT

#endif
		exit(1);
		}
	}
#ifdef __cplusplus
}
#endif



