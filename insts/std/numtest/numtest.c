#include <ugens.h>
#include <sfheader.h>
#include <stdio.h>

extern SFHEADER      sfdesc[NFILES];

/* numtest: write a test pattern into a soundfile
 
   p0 = start time
   p1 = duration
   p2 = target number
   p3 = channel 1 target number [optional]
   p4 = increment (If non-zero, don't write a single nunmber; instead, start
        from zero, add <increment> to successive samples until reaching the
        target number, return to <increment>, then do it over and over,
        creating an upward sawtooth wave.) [optional]
*/
double numtest(float p[], int n_args)
{
	int	i, nsamps;
	float	num0, num1, startnum0, startnum1, endnum0, endnum1, increment;
	float	out[2];

	nsamps = setnote(p[0],p[1],1);
	increment = p[4];

	startnum0 = endnum0 = p[2];
	startnum1 = endnum1 = (n_args > 3) ? p[3] : p[2];
	if (increment > 0.0) {
		startnum0 = 0.0;
		startnum1 = 0.0;
	}
	num0 = startnum0;
	num1 = startnum1;

	for (i = 0; i < nsamps; i++ ) {
		out[0] = num0;
		out[1] = num1;
		ADDOUT(out, 1);

		if (increment > 0.0) {
			num0 += increment;
			if (num0 > endnum0)
				num0 = increment;
			num1 += increment;
			if (num1 > endnum1)
				num1 = increment;
		}
	}
	endnote(1);

	return 0.0;
}


int profile()
{
	UG_INTRO("numtest", numtest);
	return 0;
}

