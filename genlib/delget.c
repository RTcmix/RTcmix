
float delget(float *a, float wait, int *l)
{
/*  get value from delay line, wait seconds old. */

	register int i = l[0] - (int)(wait * l[1] +.5);
	if(i < 0)  {
		i += l[2];
		if(i < 0) return 0;
		}
	return a[i];
}
