
float dliget(float *a, float wait, int *l)
{
/* get interpolated value from delay line, wait seconds old */
	register int im1;
	float x = wait * l[1];
	register int i = (int) x;
	float frac = x - i;
	i = l[0] - i;
	im1 = i - 1;
	if(i <= 0) { 
		if(i < 0) i += l[2];
		if(i < 0) return 0;
		if(im1 < 0) im1 += l[2];
		}
	return a[i] + frac * (a[im1] - a[i]);
}
