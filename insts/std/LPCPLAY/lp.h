/* LP.H	*/

/* constants used in lpc code */

#include <rt_types.h>
#include <lpcdefs.h>

/* where we call makegen() to store the private tables we need */

#define SINE_SLOT	25
#define ENV_SLOT	26

#ifdef __cplusplus
extern "C" {
#endif

/*
   Declarations of all functions which might be used globally
*/
double shift(float, float, float);
void bmultf(float *array, float mult, int number);
int stabilize(float *array, int npoles);

/* temporary until compiler bug fixed */
void l_srrand(unsigned x);
void l_brrand(float amp, float *a, int j);

#ifdef __cplusplus
}
#endif
