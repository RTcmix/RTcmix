/* p0,2,4,5,6,8,10.. are times, p1,3,5,7,9,11.. are amps, total number of
 * arguments is n_args, result is stuffed into array array of length length
 */

#include <stdio.h>

setline(float *p, short n_args,int length,float *array)
{
	double increm;
	int i,j,k,points;

	if((n_args % 2) != 0) {
		fprintf(stderr,"Something wrong with phrase, check args\n");
		closesf();
	}

	increm = (double)(p[n_args - 2] - p[0])/(double)length;
	for(j=0,i=0; j < (n_args-2); j += 2) {
		points = (int)((double)(p[j+2] - p[j]) / increm +.5);
		if(p[j+2] != p[j]) {
			if(points <= 0) points = 1;
			if((p[j+2] < p[j]) || (points > length)) {
				fprintf(stderr," confusion on phrase card\n");
				closesf();
				}
			for(k=0; k < points; k++) {
				array[i++] = ((float)k/(float)points)
					* (p[j+3] - p[j+1]) + p[j+1];
				if(i == length) return;
			}
		}
	}
	i--;
	while(++i < length) array[i] = array[i-1]; 
}
