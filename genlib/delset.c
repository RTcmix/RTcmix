/* delay initialization.  a is address of float array, l is array of 3 ints
   for bookkeeping variables, xmax, is maximum expected delay
*/
void
delset(float SR, float *a, int *l, float xmax)
{
	int i;
	l[0] = 0;
	l[1] = SR;
	l[2] = (int)(xmax * SR + .5);
	for (i = 0; i < l[2]; i++) a[i] = 0;
}
