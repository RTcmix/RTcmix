#include <ugens.h>
#include <sfheader.h>
#include <stdio.h>

extern SFHEADER      sfdesc[NFILES];

/* numtest: write a single number repeatedly into a soundfile
*
*  p0=start_time
*  p1=duration
*  p2=the number
*  p3=channel 1 number [optional]
*/

double numtest(p,n_args)
float p[];
int n_args;
{
	int i,nsamps;
	float out[2];

	nsamps = setnote(p[0],p[1],1);

	for (i = 0; i < nsamps; i++ ) {
		out[0] = out[1] = p[2];
		if (n_args > 3) out[1] = p[3];
		ADDOUT(out, 1);
		}
	endnote(1);
}


profile()
{
	UG_INTRO("numtest",numtest);
}

