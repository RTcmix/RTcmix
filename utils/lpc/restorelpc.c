
#include <sys/file.h>
#include <stdio.h>

#ifdef USE_HEADERS
int inlphs=0, outlphs=0;
#else
#define inlphs (0)
#define outlphs (0)
#endif


main(argc,argv)
char *argv[];
{
	int in,out,startrestore,endrestore,i,nbytes,nread,npoles=0,frsize;
	char *input,*output;
	float buffer[10000];
	
	
	if(argc != 6) {
		printf("usage: restorelpc inputlpc outputlpc framefirst framelast npoles\n");
		exit(0);
	}
	input=argv[1];
	output=argv[2];
	startrestore = atoi(argv[3]);
	endrestore = atoi(argv[4]);
	npoles = atoi(argv[5]);
	frsize = (npoles+4)*4;
	printf("%d %d\n",startrestore,endrestore);
	if((in=open(input,2)) < 0) {
		printf("cant open inputfile \n");
		exit(-1);
		}
	if((out=open(output,2)) < 0) {
		printf("cant open outputfile\n");
		exit(-1);
		}
#ifdef USE_HEADERS
	if((inlphs = checkForHeader(in, &npoles, 0.0)) < 0) exit(1);
	if((outlphs = checkForHeader(out, &npoles, 0.0)) < 0) exit(1);
#endif
	if((lseek(in,inlphs+startrestore*frsize,0)) < 0) {
		printf("bad lseek on inputfile\n");
		exit(-1);
		}
	if((lseek(out,outlphs+startrestore*frsize,0)) < 0) {
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
