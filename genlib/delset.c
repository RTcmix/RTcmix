
void
delset(float SR, float *a, int *l, float xmax)
{
/* delay initialization.  a is address of float array, l is size+3 int 
 * array for bookkeeping variables, xmax, is maximum expected delay */

	int i;
	l[0] = 0;
	l[1] = SR;
	l[2] = (int)(xmax * SR + .5);
	for(i = 0; i < l[2]; i++) a[i] = 0;
}
