#include "../H/sfheader.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
static SFCODE	ampcode = {
	SF_MAXAMP,
	sizeof(SFMAXAMP) + sizeof(SFCODE)
}; 

extern int swap;

main(argc,argv)
int argc;
char **argv;
{
	int sf,result;
	struct stat sfst;
	long dur;
	SFHEADER sfh;
	char *sfname;
	double atof(),amount;

	if(argc != 3) {
		printf("format: sfshrink  new_duration_in_seconds filename\n");
		exit(-1);
	}
	amount = atof(*++argv);
	sfname = *++argv;
	rwopensf(sfname,sf,sfh,sfst,"sfshrink",result,2)
	if(result < 0) exit(-1);
	dur = amount * sfsrate(&sfh) * sfclass(&sfh) 
		* sfchans(&sfh) + getheadersize(&sfh);
	dur -= dur % (sfclass(&sfh) * sfchans(&sfh));
	if(ftruncate(sf,(long)dur) < 0) 
		printf("Bad truncation\n");
	putlength(sfname,sf,&sfh);
	close(sf);
}
