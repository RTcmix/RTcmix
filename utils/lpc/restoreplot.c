
#include <sys/file.h>
#include <stdio.h>




main(argc,argv)
char *argv[];
{
	int in,out,startrestore,endrestore,i,nbytes,nread,npoles,frsize;
	char *input,*output;
	float buffer[10000];
	
	
	if(argc != 5) {
		printf("usage: restorelpc inputlpc outputlpc framefirst framelast\n");
		exit(0);
	}
	input=argv[1];
	output=argv[2];
	startrestore = atoi(argv[3]);
	endrestore = atoi(argv[4]);
	frsize = 8;
	printf("%d %d\n",startrestore,endrestore);
	if((in=open(input,2)) < 0) {
		printf("cant open inputfile \n");
		exit(-1);
		}
	if((out=open(output,2)) < 0) {
		printf("cant open outputfile\n");
		exit(-1);
		}
	if((lseek(in,startrestore*frsize,0)) < 0) {
		printf("bad lseek on inputfile\n");
		exit(-1);
		}
	if((lseek(out,startrestore*frsize,0)) < 0) {
		printf("bad lseek on outputfile");
		exit(-1);
	}
	nbytes = (endrestore - startrestore + 1) * frsize;
	printf("nbytestoread = %d\n",nbytes);
	if(nread = read(in,(char *)buffer,nbytes) != nbytes) {
		printf("bad read on inputfile %d\n",nread);
		exit(-1);
		}
	if(nread = write(out,(char *)buffer,nbytes) != nbytes) {
		printf("bad write on outputfile %d\n",nread);
		exit(-1);
		}

}
