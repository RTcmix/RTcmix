#include <math.h>

/*---------------------------------------
	cford1.c

in-place reordering subroutine
#--------------------------------------*/

void
cford1(int m, float *b)
{
	long k, kl, n, j;
	float t;

	k = 4;
	kl = 2;
	n = pow(2.,(float)m);
	for (j = 4; j <= n; j+=2) {
		if (k>j) {
			t = b[j-1];
			b[j-1] = b[k-1];
			b[k-1] = t;
		}
		k = k-2;
		if (k<=kl) {
			k = 2*j;
			kl = j;
		}
	}
}

