#include "../H/ugens.h"
#include "../H/sfheader.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>

extern SFHEADER      sfdesc[NFILES];
 
fplot(float *p, short n_args)
{
	float out[80],si,*f1,phase;
	int i,k,j,len,wave;
	static char line[80];

	wave = p[0];
	phase = 0;
	len = fsize(wave);
	si = (float) len/79.;
	f1 = (float *) floc(wave);
	for(i = 0; i < 80; i++) {
		out[i] =  oscil(1.,si,f1,len,&phase);
	}
	si = 1./11.;
	phase = 1.;
	for(i = 0; i < 23; i++) {
		k = 0;
		for(j = 0; j < 79; j++) {
		   if((out[j]>=phase) && (out[j]<phase+si)) {
		      line[j] = (out[j+1] > phase+si) ? '/' : '-';
		      if(out[j+1] < phase) line[j] = '\\';
		      k = j;
		      }
                   else if (((out[j]<phase)&&(out[j+1]>phase+si)) ||
		      ((out[j]>phase+si)&&(out[j+1]<phase))) {
		         line[j] = '|';
			 k = j;
			 }
		   else line[j] = ' ';
		}
		if ((0>=phase) && (0<phase+si)) {
		      for(j = 0; j < 79; j++) line[j] = '-';
		      k = 78;
		      }
		line[k+1] = '\0';
		puts(line);
		phase = phase - si;
	}
}
