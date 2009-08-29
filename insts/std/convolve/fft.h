#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define FORWARD 1
#define INVERSE 0

typedef struct { float re ; float im ; } complex ;

typedef struct {
  float    min;
  float    max;
} Bound;

#define CABS(x) hypot( (x).re, (x).im )

extern complex zero ;
extern complex one ;
/* extern char *malloc(), *calloc() ; */
extern float synt ;

/* DT:  SHOULD NEVER USE PI LIKE THIS !!!! */
/* extern float PI ; */
#ifndef PI
#define PI  M_PI  /* should be in <math.h>   -JGG */
#endif
extern float TWOPI ;


/*
 * memory allocation macro
 */
#define fvec( name, size )\
if ( ( name = (float *) calloc( size, sizeof(float) ) ) == NULL) {\
    fprintf( stderr, "Insufficient memory\n" ) ;\
    exit( -1 ) ;\
}

