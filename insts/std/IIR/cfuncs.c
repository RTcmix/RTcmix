#include <stdio.h>
#include <ugens.h>

float rsnetc[64][5],amp[64];
int nresons;


double setup(float *p, int n_args)
{
	int i,j;
	float first,bw,cf;

	first = (p[0] < 15.0) ? cpspch(p[0]) : p[0];
	bw = p[1] < 0 ? -p[1] * first : p[1];
	rsnset(first,bw,1.,0.,rsnetc[0]);
	advise("IIR setup", "centerfreq    bandwidth  relative amp");
	amp[0] = p[2];
	advise(NULL, "            %10.4f %10.4f %10.4f",first,bw,amp[0]);
	for(i=3,j=1; i<n_args; i += 3)  {
	        if(p[i] < 0.) cf = -p[i] * first;
	        else  cf = (p[i] < 15.0) ? cpspch(p[i]) : p[i];
	        bw = p[i+1] < 0 ? -p[i+1] * cf : p[i+1];
	        amp[j] = p[i+2];
	        rsnset(cf ,bw ,1.,0.,rsnetc[j]);
	        advise(NULL, "            %10.4f %10.4f %10.4f",cf,bw,amp[j]);
	        j++;
	}
	nresons = j;
	return((double)nresons);
}


int
profile()
{
	UG_INTRO("setup",setup);
	return 0;
}

