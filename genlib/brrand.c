
/* block version of rrand */
/* a modification of unix rand() to return floating point values between
   + and - 1. */

static	long	randx = 1;

void
sbrrand(unsigned x)
{
	randx = x;
}

void
brrand(float amp, float *a, int j)
{
	int k;
	for(k=0; k<j; k++) {
		int i = ((randx = randx*1103515245 + 12345)>>16) & 077777;
		*a++ = amp * ((float)i/16384. - 1.);
	}
}
