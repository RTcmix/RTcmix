/*---------------------------
	cfr2tr.c

Radix 2 iteration subroutine.
---------------------------*/

void
cfr2tr(long off, float *b0, float *b1)
{
   long i;
   float tmp;

   for (i = 0; i < off; i++) {
      tmp = b0[i] + b1[i];
      b1[i] = b0[i] - b1[i];
      b0[i] = tmp;
   }
}

