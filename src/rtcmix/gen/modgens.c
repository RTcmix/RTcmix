/* RTcmix  - Copyright (C) 2002  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>  /* for memmove */
#include <float.h>   /* for FLT_MIN and FLT_MAX */
#include <math.h>    /* for rintf */
#include <ugens.h>


/* -------------------------------------------------------------- addgens --- */
/* Add corresponding values of gens whose table numbers are given in
   p1 and p2, placing the result in a new gen in the table number given in
   p0.  If the input tables are not the same size, the smaller one is resampled
   to have the size of the larger one before adding corresponding values.
*/
double
m_addgens(float p[], int n_args, double pp[])
{
   int   destslot, srcslot1, srcslot2, normalize, size;

   destslot = (int) p[0];
   srcslot1 = (int) p[1];
   srcslot2 = (int) p[2];
   normalize = (int) p[3];

   size = combine_gens(destslot, srcslot1, srcslot2, normalize,
                                                ADD_GENS, "addgens");
   return (double) size;
}


/* ------------------------------------------------------------- multgens --- */
/* Multiply corresponding values of gens whose table numbers are given in
   p1 and p2, placing the result in a new gen in the table number given in
   p0.  If the input tables are not the same size, the smaller one is resampled
   to have the size of the larger one before multiplying corresponding values.
*/
double
m_multgens(float p[], int n_args, double pp[])
{
   int   destslot, srcslot1, srcslot2, normalize, size;

   destslot = (int) p[0];
   srcslot1 = (int) p[1];
   srcslot2 = (int) p[2];
   normalize = (int) p[3];

   size = combine_gens(destslot, srcslot1, srcslot2, normalize,
                                                MULT_GENS, "multgens");
   return (double) size;
}


/* -------------------------------------------------------------- copygen --- */
/* Make a copy of the gen whose table number is given in p1.  Assign the
   copy the table number given in p0.  The copied table will have the size
   given in p2.  p3 is a number specifying the type of interpolation to use
   when resampling the source table to fit the new number of locations.
   Return the size of the new table.

      p0    table number of new table
      p1    table number of original table
      p2    size of new table [optional, default is size of original]
      p3    interpolation type (0: no interpolation, 1: linear interpolation)
               [optional, default is 1]
*/
double
m_copygen(float p[], int n_args, double pp[])
{
   int   srcslot, destslot, srcsize, destsize;
   float *srctable, *desttable;
   InterpolationType interp;

   destslot = (int) p[0];
   srcslot = (int) p[1];
   srctable = floc(srcslot);
   if (srctable == NULL)
      die("copygen", "No function table defined for slot %d.", srcslot);
   srcsize = fsize(srcslot);

   destsize = (n_args > 2) ? (int) p[2] : srcsize;
   interp = (n_args > 3) ? (InterpolationType) p[3] : LINEAR_INTERP;

   desttable = resample_gen(srctable, srcsize, destsize, interp);
   if (desttable == NULL)
      die("copygen", "No memory to copy the gen in slot %d.", srcslot);

   if (!install_gen(destslot, destsize, desttable))
      die("copygen", "No more function tables available.");

   return (double) destsize;
}


/* ------------------------------------------------------------ offsetgen --- */
/* Add a constant, given in p1, to the values of the gen whose table number
   is given in p0.  Note that no rescaling of the resulting table is done,
   so that values outside [-1, 1] are possible.
*/
double
m_offsetgen(float p[], int n_args, double pp[])
{
   int   i, slot, size;
   float *table, offset;

   slot = (int) p[0];
   table = floc(slot);
   if (table == NULL)
      die("offsetgen", "No function table defined for slot %d.", slot);
   size = fsize(slot);
   offset = p[1];

   for (i = 0; i < size; i++)
      table[i] = table[i] + offset;

   return (double) size;
}


/* ------------------------------------------------------------- scalegen --- */
/* Multiply by a constant, given in p1, the values of the gen whose table
   number is given in p0.  Note that no rescaling of the resulting table is
   done, so that values outside [-1, 1] are possible.
*/
double
m_scalegen(float p[], int n_args, double pp[])
{
   int   i, slot, size;
   float *table, scale;

   slot = (int) p[0];
   table = floc(slot);
   if (table == NULL)
      die("scalegen", "No function table defined for slot %d.", slot);
   size = fsize(slot);
   scale = p[1];

   for (i = 0; i < size; i++)
      table[i] = table[i] * scale;

   return (double) size;
}


/* ------------------------------------------------------------ invertgen --- */
/* Invert the values of the gen whose table number is given in p0.  The
   y-axis center of symmetry is a point halfway between the min and max
   table values; inversion is performed around this center of symmetry.
*/
double
m_invertgen(float p[], int n_args, double pp[])
{
   int   i, slot, size;
   float min, max, center, *table;

   slot = (int) p[0];
   table = floc(slot);
   if (table == NULL)
      die("invertgen", "No function table defined for slot %d.", slot);
   size = fsize(slot);

   /* determine central y-axis value */
   min = FLT_MAX;
   max = FLT_MIN;
   for (i = 0; i < size; i++) {
      if (table[i] < min)
         min = table[i];
      if (table[i] > max)
         max = table[i];
   }
   center = min + ((max - min) / 2.0);

//advise("invertgen", "min: %f, max: %f, center: %f", min, max, center);

   /* invert values around center */
   for (i = 0; i < size; i++) {
      float diff = table[i] - center;
      table[i] = center - diff;
   }

   return (double) size;
}


/* ----------------------------------------------------------- reversegen --- */
/* Reverse the values of the gen whose table number is given in p0. */
double
m_reversegen(float p[], int n_args, double pp[])
{
   int   i, j, slot, size;
   float *table;

   slot = (int) p[0];
   table = floc(slot);
   if (table == NULL)
      die("reversegen", "No function table defined for slot %d.", slot);
   size = fsize(slot);

   for (i = 0, j = size - 1; i < size / 2; i++, j--) {
      float temp = table[i];
      table[i] = table[j];
      table[j] = temp;
   }

   return (double) size;
}


/* ------------------------------------------------------------ rotategen --- */
/* Rotate the values of the gen whose table number is given in p0 by the number
   of array locations given in p1.  Positive values rotate to the right;
   negative values to the left.  If a value is rotated off the end of the
   array in either direction, it reenters the other end of the array.
*/
double
m_rotategen(float p[], int n_args, double pp[])
{
   int      slot, size, rotate, absrotate;
   size_t   movesize;
   float    *table, *scratch;

   slot = (int) p[0];
   table = floc(slot);
   if (table == NULL)
      die("rotategen", "No function table defined for slot %d.", slot);
   size = fsize(slot);
   rotate = (int) p[1];
   if (rotate == 0) {
      advise("rotategen", "You're rotating by 0 locations!");
      return (double) size;
   }
   absrotate = abs(rotate);
   if (absrotate >= size)
      die("rotategen", "Rotate value must be less than table size.");

   scratch = (float *) malloc((size_t)(absrotate * sizeof(float)));
   if (scratch == NULL)
      die("rotategen", "No memory for scratch buffer!");

   /* an example of what should happen...
      [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]      src table, size = 10
      [7, 8, 9, 0, 1, 2, 3, 4, 5, 6]      dest, rotate = 3
      [3, 4, 5, 6, 7, 8, 9, 0, 1, 2]      dest, rotate = -3
   */
   movesize = (size_t) (size - absrotate);  /* floats to shift within table */
   if (rotate > 0) {
      memcpy(scratch, table + movesize, (size_t) absrotate * sizeof(float));
      memmove(table + absrotate, table, movesize * sizeof(float));
      memcpy(table, scratch, (size_t) absrotate * sizeof(float));
   }
   else {
      memcpy(scratch, table, (size_t) absrotate * sizeof(float));
      memmove(table, table + absrotate, movesize * sizeof(float));
      memcpy(table + movesize, scratch, (size_t) absrotate * sizeof(float));
   }

   free(scratch);

   return (double) size;
}


/* ---------------------------------------------------------- quantizegen --- */
/* Quantize the values of the gen whose table number is given in p0 to the
   quantum given in p1.
*/
double
m_quantizegen(float p[], int n_args, double pp[])
{
   int      i, slot, size;
   float    quantum, *table;

   slot = (int) p[0];
   table = floc(slot);
   if (table == NULL)
      die("quantizegen", "No function table defined for slot %d.", slot);
   size = fsize(slot);
   quantum = p[1];

   for (i = 0; i < size; i++) {
      float q = rintf(table[i] / quantum);
      table[i] = q * quantum;
   }

   return (double) size;
}


