#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#define FLOAT 4
main(argc,argv)
int argc; 
char *argv[];
{
        int anal,pitches,j,jj;
	int npoles,lpcframe,pchframe,pchlast,nbpch;
	int correct_();
	int nskiplpc,nskippch,nbytes,nblpc;
	char  input[32],*output;
	float pch[2],val[1];
	float y[50],frame[50],new;
	int out,i;
	int flag,framenum;
	/*
	printf(" Enter name of lpc analysis file\t");
	scanf("%s",output);
	*/
	output = argv[1];
	if((anal = open(output,2)) < 0) {
		fprintf(stderr," Can't open lpc analysis file");
		exit(1);
		}
flag = 1;
	if((out = open("badframes",O_CREAT|O_RDWR, 0644)) < 0) {
		fprintf(stderr,"Can't open outputfile\n");
		exit(-2);
		}
printf("anal and out = %d %d %d\n",anal,out,sizeof(framenum));
/*
	printf(" Enter number of poles in lpc analysis\t");
	scanf("%d",&npoles);
*/
	npoles = atoi(argv[2]);
	printf("npoles = %d\n",npoles);
	nblpc = (npoles+4)*FLOAT;/*to beginning of next pchloc*/
	framenum = 0;
	lseek(anal,0,0);
	nbytes = 4;
	while(1) {
		if((read(anal,(char *)frame,nblpc)) != nblpc) {
			printf("Bad read on lpc analysis file\n");
			printf("nblpc = %d %d\n",nblpc,jj);
			exit(1);
			}
		for(i=0; i<npoles; i++) y[i] = -frame[npoles+3-i];
		stabletest_(y,&npoles,&flag);
		if(!flag) printf("flag,framenum %d %d\n",flag,framenum);
		if(!flag) if(jj=write(out,&framenum,nbytes) != nbytes) {
			printf("bad write on output file %d\n",jj);
			exit(-1);
			}

/*
if(!flag){ printf("tobefixed\n"); for(i=0; i<npoles+4; i++) printf(" %f",frame[i]);
printf("\n");
}
*/
if(!flag) printf("fixing frame number %d\n",framenum);
		if(!flag) {
		/*
printf("before correction npoles = %d\n",npoles);
for(i=0; i<npoles+4; i++) printf(" %f",frame[i]);
*/
		correct_(frame,&npoles,y);
for(i=4; i<npoles+4; i++) frame[i] = y[i-4]; 
/*
printf("after correction\n");
for(i=0; i<npoles+4; i++) printf(" %f",frame[i]);
printf("\n");
*/
		lseek(anal,-nblpc,1);
		if((write(anal,(char *)frame,nblpc)) != nblpc) {
			printf("Bad write on lpc analysis file\n");
			printf("nblpc = %d %d\n",nblpc,jj);
			exit(1);
			}

	}
	framenum++;
	fflush(stdout);
	}
}
