/* fast file-to-file copy */
/* p0=input fno, p1=output fno, p2=inputskip, p3=output skip, p4=dur */

#include "../H/ugens.h"
#include "../H/sfheader.h"
#include <stdio.h>

extern int  sfd[NFILES];            /* soundfile descriptors */
extern char *sndbuf[NFILES];        /* address of buffer */
extern long  filepointer[NFILES];   /* to save current pointer in file */
extern int   NBYTES;


extern SFHEADER      sfdesc[NFILES];

sfcopy(p,n_args)
float *p;
{
	int maxread,n,input,output,nbytes,jj;

	if(!n_args) fprintf(stderr,"(sfcopy(input_fno,output_fno,input_skip,output_skip,dur)\n");
	input = (int)p[0];
	output = (int)p[1];
	if((sfclass(&sfdesc[input]) != sfclass(&sfdesc[output])) ||
	   (sfchans(&sfdesc[input]) != sfchans(&sfdesc[output]))) {
		fprintf(stderr,
		 "Input and output specifications do not match. Canot copy.\n");
		closesf();
	}
	nbytes = setnote(p[2],p[4],input) * 
		 sfchans(&sfdesc[input]) * sfclass(&sfdesc[input]);

	setnote(p[3],p[4],output);
	_backup(input);
	_backup(output);

	fprintf(stderr,"Copy %d bytes\n",nbytes);

	while(nbytes) {
		maxread = (nbytes > NBYTES) ? NBYTES : nbytes;
		if((n = read(sfd[input],sndbuf[input],maxread)) <= 0) {
			fprintf(stderr,"Apparent eof on input\n");
			return;
		}
		if((jj = write(sfd[output],sndbuf[input],n)) <= 0) {
			fprintf(stderr,"Trouble writing output file\n");
			closesf();
		}
		nbytes -= n;
		filepointer[input] += n;
		filepointer[output] += n;
	}
	if(fsync(sfd[output]) < 0 ) {
		fprintf(stderr,"trouble fsyncing file");
		closesf();
	}
	fprintf(stderr,"Copy completed\n");
}
