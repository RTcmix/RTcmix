void
delput(float x, float *a, int *l)
{

/* put value in delay line. See delset. x is float */

	*(a + (*l)++) = x;
	if(*(l) >= *(l+1)) *l -= *(l+1);
}
