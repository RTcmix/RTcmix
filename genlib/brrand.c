
/* block version of rrand */
/* a modification of unix rand() to return floating point values between
   + and - 1. */

static	int	randx = 1;

void
sbrrand(unsigned int x)
{
	randx = x;
}

void
brrand(float amp, float *a, int j)
{
	int i, k;
	for(k=0; k<j; k++) {
		i = ((randx = randx*1103515245 + 12345)>>16) & 077777;
		a[k] = amp * ((float)i/16384.0f - 1.0f);
	}
}
