#include "../H/ugens.h"
#include <stdio.h>

extern double gen2(struct gen *gen);
extern double gen3(struct gen *gen);
extern double gen5(struct gen *gen);
extern double gen6(struct gen *gen);
extern double gen7(struct gen *gen);
extern double gen9(struct gen *gen);
extern double gen10(struct gen *gen);
extern double gen17(struct gen *gen);
extern double gen18(struct gen *gen);
extern double gen20(struct gen *gen);
extern double gen24(struct gen *gen);
extern double gen25(struct gen *gen);


static int ngens = 1;           /* total number of gens so far */
/* Note: This was 0, but then non-existant gen funcs were not caught.  -JGG */

/* ok, TOTGENS is the absolute total number of gens we can have in
   a given run, MAXGENS is the number of simultaneous gens we can
   have.  TOTGENS should probably be replaced by an appropriate malloc */

#define TOTGENS 10000
#define MAXGENS 300

float *farrays[TOTGENS];
int sizeof_farray[TOTGENS];
int f_goto[MAXGENS];   /* this is used to guarantee unique farrays in rtcmix */


/* p0=storage loc, p1=gen no, p2=size, p3--> = args */
double
makegen(float p[], int n_args)
{
   float  *block;
   double retval;
   struct gen gen;
   char *valloc();

   int genno = p[0];
   if (genno < 0)
      genno = -genno;

   if (genno >= MAXGENS) {
      fprintf(stderr, "makegen: no more simultaneous gens available!\n");
      closesf();
   }

   /* makegen now creates a new function for *every* call to it - this
      is so that we can guarantee the correct version of a given function
      at run-time during RT operation.  the f_goto[] array keeps track
      of where to map a particular function table number during the
      queueing of RT Instruments
   */
   f_goto[genno] = ngens;

   gen.size = p[2];

// FIXME: Should we really be using valloc here?  -JGG
   block = (float *) valloc((unsigned) gen.size * FLOAT);
   if (block == NULL) {
      fprintf(stderr, "makegen: Can't get any function space.\n");
      closesf();
   }
   farrays[f_goto[genno]] = block;
   ngens++;

   sizeof_farray[f_goto[genno]] = p[2];

   gen.nargs = n_args - 3;
   gen.pvals = p + 3;
   gen.array = farrays[f_goto[genno]];
   gen.slot = (int) p[0];

   switch ((int) p[1]) {
      case 25:
         retval = gen25(&gen);
         break;
      case 24:
         retval = gen24(&gen);
         break;
      case 20:
         retval = gen20(&gen);
         break;
      case 18:
         retval = gen18(&gen);
         break;
      case 17:
         retval = gen17(&gen);
         break;
      case 10:
         retval = gen10(&gen);
         break;
      case 9:
         retval = gen9(&gen);
         break;
      case 7:
         retval = gen7(&gen);
         break;
      case 6:
         retval = gen6(&gen);
         break;
      case 5:
         retval = gen5(&gen);
         break;
      case 3:
         retval = gen3(&gen);
         break;
      case 2:
         retval = gen2(&gen);
         break;
      default:
         fprintf(stderr, "makegen: There is no gen%d\n", (int) p[1]);
         retval = -1.0;
   }

   return retval;
}


