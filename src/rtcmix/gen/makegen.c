/* RTcmix  - Copyright (C) 2002  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <ugens.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>    /* for fabs */
#include <assert.h>

extern double gen1(struct gen *gen, char *sfname);
extern double gen2(struct gen *gen);
extern double gen3(struct gen *gen);
extern double gen4(struct gen *gen);
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


/* -------------------------------------------------------------- makegen --- */
/* p0=storage loc, p1=gen no, p2=size, p3--> = args */
double
makegen(float p[], int n_args, double pp[])
{
   int    genslot, genno;
   double retval = -1.0;
   struct gen gen;

   genslot = (int) p[0];
   genno = (int) p[1];

   if (genslot < 0)
      genslot = -genslot;
   if (genslot >= MAXGENS)
      die("makegen", "no more simultaneous gens available!");

   /* makegen now creates a new function for *every* call to it - this
      is so that we can guarantee the correct version of a given function
      at run-time during RT operation.  the f_goto[] array keeps track
      of where to map a particular function table number during the
      queueing of RT Instruments
   */
   f_goto[genslot] = ngens;

   gen.size = p[2];

// FIXME: Should we really be using valloc here?  -JGG
   if (genno != 1) {    /* gen1 must allocate its own memory */
      float *block = (float *) valloc((unsigned) gen.size * FLOAT);
      if (block == NULL)
         die("makegen", "Not enough memory for function table %d.", genslot);
      farrays[f_goto[genslot]] = block;
   }
   ngens++;

   sizeof_farray[f_goto[genslot]] = p[2];

   gen.nargs = n_args - 3;
   gen.pvals = p + 3;
   gen.array = farrays[f_goto[genslot]];
   gen.slot = (int) p[0];   /* get from pfield, to preserve negative "flag" */

   switch (genno) {
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
      case 4:
         retval = gen4(&gen);
         break;
      case 3:
         retval = gen3(&gen);
         break;
      case 2:
         retval = gen2(&gen);
         break;
      case 1:
         {
            char *sfname = (char *) ((int) pp[3]);
            gen.pvals = p + 2;    /* gen1() needs size pfield */
            gen.nargs++;
            retval = gen1(&gen, sfname);
            if (retval != -1.0) {
               /* We don't know these before calling gen1, so get them now. */
               farrays[f_goto[genslot]] = gen.array;
               sizeof_farray[f_goto[genslot]] = gen.size;
            }
         }
         break;
      default:
         die("makegen", "There is no gen%d.", (int) p[1]);
   }

   return retval;
}


/* --------------------------------------------------------- combine_gens --- */
int
combine_gens(int destslot, int srcslot1, int srcslot2, int normalize,
                                    GenModType modtype, char *funcname)
{
   int      i;
   int      destindex, srcindex1, srcindex2;
   int      destsize, srcsize1, srcsize2;
   float    *destarray, *srcarray1, *srcarray2, *tmparray = NULL;

   assert(modtype == ADD_GENS || modtype == MULT_GENS);

   if (destslot == 0 || srcslot1 == 0 || srcslot2 == 0)
      die(funcname, "Gen number pfields cannot be zero.");
   if (srcslot1 >= MAXGENS)
      die(funcname, "Gen number %d out of range.", srcslot1);
   if (srcslot2 >= MAXGENS)
      die(funcname, "Gen number %d out of range.", srcslot2);
   if (destslot >= MAXGENS)
      die(funcname, "No more simultaneous function tables available.");

   srcindex1 = f_goto[srcslot1];
   srcindex2 = f_goto[srcslot2];
   srcsize1 = sizeof_farray[srcindex1];
   srcsize2 = sizeof_farray[srcindex2];
   if (srcsize1 == 0)
      die(funcname, "Gen number %d doesn't exist.", srcslot1);
   if (srcsize2 == 0)
      die(funcname, "Gen number %d doesn't exist.", srcslot2);

   srcarray1 = farrays[srcindex1];
   srcarray2 = farrays[srcindex2];

   /* If necessary, resample shorter table so that it uses same number of
      slots as the longer table.  (Code came from objlib/NZero.C.)
   */
   if (srcsize1 != srcsize2) {
      int   oldsize;
      float *oldarray, incr, f, frac, next, diff = 0.0;

      if (srcsize1 > srcsize2) {
         destsize = srcsize1;
         oldsize = srcsize2;
         oldarray = srcarray2;
      }
      else {
         destsize = srcsize2;
         oldsize = srcsize1;
         oldarray = srcarray1;
      }
      tmparray = (float *) malloc((size_t) destsize * sizeof(float));
      if (tmparray == NULL)
         die(funcname, "Not enough memory for temporary function table.");

      incr = (float) oldsize / (float) destsize;
      f = 0.0;
      for (i = 0; i < destsize; i++) {
         int n = (int) f;
         frac = f - (float) n;
         if (frac) {
            next = (n + 1 < oldsize) ? oldarray[n + 1] : oldarray[oldsize - 1];
            diff = next - oldarray[n];
         }
         tmparray[i] = oldarray[n] + (diff * frac);
         f += incr;
      }

      if (srcsize1 > srcsize2)
         srcarray2 = tmparray;
      else
         srcarray1 = tmparray;
   }
   else
      destsize = srcsize1;

   destarray = (float *) malloc((size_t) destsize * sizeof(float));
   if (destarray == NULL)
      die(funcname, "Not enough memory for new function table.");

   /* Fix bookkeeping vars. */
   destindex = ngens++;
   f_goto[destslot] = destindex;
   sizeof_farray[destindex] = destsize;
   farrays[destindex] = destarray;

   /* Fill destination array. */
   switch (modtype) {
      case ADD_GENS:
         for (i = 0; i < destsize; i++)
            destarray[i] = srcarray1[i] + srcarray2[i];
         break;
      case MULT_GENS:
         for (i = 0; i < destsize; i++)
            destarray[i] = srcarray1[i] * srcarray2[i];
         break;
      default:
         break;
   }

   if (tmparray)
      free(tmparray);

   if (normalize) {        /* cf fnscl.c */
      float max = 0.0;

      for (i = 0; i < destsize; i++) {
         float val = fabs(destarray[i]);
         if (val > max)
            max = val;
      }
      for (i = 0; i < destsize; i++)
         destarray[i] /= max;
   }

   return destsize;
}


