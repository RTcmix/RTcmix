#include <stdlib.h>
#include <stdio.h>
#include "../H/ugens.h"

extern FILE *infile_desc[50];   /* contains file descriptors for data files */


/* gen2 lets the user fill a function table with numbers specified in the
   score or in a text file. There are two "API's" for this: the original
   one and a newer one that fixes problems with the older one. The old
   one remains for backwards compatibility with scores, as well as for
   the data file input functionality.

   Old way:

      makegen(x, 2, tablesize, fnum)

      If fnum is 0, then the table is filled with numbers appearing on
      the next line in the score. The gen tries to grab <tablesize>
      numbers. An example...

      makegen(1, 2, 8, 0)
      1.2  4.3  9.8  4.5  6.2  8.3  1.9  1.0

      If fnum is > 0, then it identifies a text file already opened
      by the infile Minc call, which returns this id. (The id is an
      index into a table of FILE pointers.)

   New way:

      The old way has problems. When used with fnum=0, it reads from
      stdin, behind the back of the parser. This means you can't use
      variables as part of the list of numbers, and the makegen won't
      work if it appears in a code block. Also, if the line of numbers
      ends with a space char, gen2 often fails.

      The new way is meant to make gen2 more like other makegens:

      makegen(x, 2, tablesize, num1, num2 [, num3, ...] )

      Only <tablesize> numbers will go into the function table,
      regardless of how many arguments there are. If there are
      fewer than <tablesize> args, the remaining table slots
      are set to zero.

   Note that this code distinguishes between the old and new ways
   by the number of pfields. If there are only 4 pfields (i.e., 
   gen->nargs is 1), then it assumes the old way. Otherwise, it
   assumes the new way. So a new-style array of 1 element will
   be interpreted as an old-style array (taking values either
   from the next line or from a file).

                             [new way and comments by JGG, 21-Feb-00]
*/
double
gen2(struct gen *gen)
{
   int i;

   if (gen->nargs > 1) {        /* new way */
      int slots;

      /* gen->size is size of array */
      if (gen->size < gen->nargs)
         slots = gen->size;
      else
         slots = gen->nargs;

      for (i = 0; i < slots; i++)
         gen->array[i] = gen->pvals[i];

      while (i < gen->size)     /* fill remainder (if any) with zeros */
         gen->array[i++] = 0.0;
   }
   else {                       /* old way */
      float val;
      char  inval[128];
      FILE  *in_desc;

      /* input datafile is stdin if pval[0] = 0 */
      if (gen->pvals[0] == 0)
         in_desc = stdin;
      else
         in_desc = infile_desc[(int) gen->pvals[0]];

      if (in_desc == NULL) {       /* Stop if infile seek failed */
         fprintf(stderr, "Input error. Gen02 exited.\n");
         return -1.0;
      }

      i = 0;
      if (gen->pvals[0] == 0) {    /* if reading from stdin */
         while (fscanf(in_desc, "%f", &val) != EOF) {
            if (i < gen->size)
               gen->array[i] = val;
            i++;
            if (getc(in_desc) == 10)
               break;
         }
      }
      else {         /* if reading from input text file specified with infile */
         while (fscanf(in_desc, "%s", inval) != EOF) {
            if (i >= gen->size)
               break;
            gen->array[i] = atof(inval);
            i++;
         }
      }
      if (i > gen->size)
         fprintf(stderr, "Out of array space in gen02!!\n");

      printf("%d values loaded into array.\n",
                                          (i <= gen->size) ? i : gen->size);

      i--;
      while (++i < gen->size)      /* fill remainder (if any) with zeros */
         gen->array[i] = 0.0;
   }

   return 0.0;
}


