#include <math.h>
#include <sndlibsupport.h>  /* must come after math.h, or else M_PI redefined */
#include <byte_routines.h>
#include <sfheader.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <errno.h>

static SFCODE	ampcode = {
	SF_MAXAMP,
	sizeof(SFMAXAMP) + sizeof(SFCODE)
	}; 

static SFCODE	commentcode = {
	SF_COMMENT,
	MINCOMM + sizeof(SFCODE)
	};

int
main(int argc, char *argv[])
{
	int sf;
	int comment = 0;
	struct stat sfst;
	SFHEADER sfh;
	SFMAXAMP sfm;
	SFCOMMENT sfcm;
	SFCODE *sizer;
	FILE *fcom;
	char *sfname,*cp,*getsfcode();
	int length,newchans,newclass,newpeak,result,i,n,tfd,tn,nchars=MINCOMM;
	float newsrate;
	int zap;

	zap = newchans = newclass = newsrate = newpeak = 0;
	length = 1;

	if(argc > 1) {
		if(*argv[1] != '-') {
			printf("usage: \"sfhedit -r [srate] -[i=int;f=float] -c [nchans] -p [peak] -w [comment] -z\"\n");
			exit(1);
		}
	}
	else if(argc == 1) {
		printf("usage: \"sfhedit -r [srate] -[i=int;f=float] -c [nchans] -p [peak] -w [comment] -z\"\n");
		exit(1);
	}

	while((*++argv)[0] == '-') {
		argc -= 2; /* Take away two args */
		for(cp = argv[0]+1; *cp; cp++) {
			switch(*cp) { /* Grap options */
			case 'r': 
				newsrate = atof(*++argv);
				break;
			case 'i': 
				newclass = SF_SHORT;
				break;
			case 'f':
				newclass = SF_FLOAT;
				break;
			case 'c': 
				newchans = atoi(*++argv);
				if(newchans > 4) {
					printf(" Sorry, maximum is 4 channels\n");
					exit(1);
				}
				break;
			case 'p':
				newpeak = 1;
				break;
			case 'w':
				comment = 1;
				break;
			case 'z':
				zap = 1;
				break;
			case 'l':
				length = 1;
				break;
			default:  
				printf("don't know about option %c\n",*cp);
			}
		}
	}
	sfname = argv[0];
	rwopensf(sfname,sf,sfh,sfst,"sfhedit",result,2);
	if(result < 0) {
		exit(1);
	}
	printsf(&sfh);
	if(newchans) {
		sfchans(&sfh) = newchans;
		printf("-->Channels reset to %d\n",newchans);
	}
	if(newsrate) {
		sfsrate(&sfh) = newsrate;
		printf("-->Sampling rate reset to %5.0f\n",newsrate);
	}
	if(newclass) {
		sfclass(&sfh) = newclass;
		printf("-->Class reset to %d\n",newclass);
	}		
	if(newpeak) {
		for(i=0; i<sfchans(&sfh); i++) {
			printf("Enter peak and time for channel %d\t",i);
			scanf("%f %ld",&sfmaxamp(&sfm,i),&sfmaxamploc(&sfm,i));
		}
		sfmaxamptime(&sfm) = time(NULL);

		putsfcode(&sfh,(char *)&sfm,&ampcode);
	}
	if(zap) {
	 	if(ftruncate(sf,0) < 0) 
			printf("Bad truncation\n");
		for(i=0; i<sfchans(&sfh); i++) {
			sfmaxamp(&sfm,i) = sfmaxamploc(&sfm,i) = 0;
		putsfcode(&sfh,(char *)&sfm,&ampcode);
		putlength(sfname,sf,&sfh);
		}
		printf("file truncated to 0, and header adjusted\n");
	}
	if(comment) {
		cp = getsfcode(&sfh,SF_COMMENT);
		if(cp == NULL) {
			printf("No comment found. Adding a new one..\n");
			system("vi /tmp/comment");
			fcom = fopen("/tmp/comment","r");
			i=0;
			while ( (sfcomm(&sfcm,i) = getc(fcom)) != EOF ) {
				if (i >= MAXCOMM - 1) {
					printf("Gimme a break! I can only take %d characters\n",MAXCOMM);
					printf("comment truncated to %d characters\n",MAXCOMM);
					commentcode.bsize = MAXCOMM + sizeof(SFCODE);
					break;
				}
				++i;
			}
			sfcomm(&sfcm,i) = '\0';
			system("rm /tmp/comment");
			if (nchars > MINCOMM)
				commentcode.bsize = nchars + sizeof(SFCODE);
			if (i > nchars)
				commentcode.bsize = i + sizeof(SFCODE);
			if (putsfcode(&sfh,(char *)&sfcm,&commentcode) < 0) {
				printf("comment didn't get written, sorry!\n");
				exit(1);
			}
			goto skip;
		}

		strncpy((char *)&sfcm, sfcommentstr(&sfh), MAXCOMM - 1);
		sfcm.comment[MAXCOMM - 1] = '\0';

		tfd = open("/tmp/tmpcom",O_CREAT|O_RDWR,0644);
		tn = write(tfd, &sfcomm(&sfcm,0), strlen(&sfcomm(&sfcm,0)));
		close(tfd);
		system("vi /tmp/tmpcom");
		tfd = open("/tmp/tmpcom",0);
		n = read(tfd,&sfcomm(&sfcm,0),MAXCOMM);
		system("rm /tmp/tmpcom");
		if (n < tn) {
			for (i = n; i <= tn; i++) {
				sfcomm(&sfcm,i) = '\0';
			}
		}
		if (putsfcode(&sfh,(char *)&sfcm,&commentcode) < 0) {
			printf("comment didn't get written, sorry!\n");
			exit(1);
		}
	}

skip:	lseek(sf,0,0);
	if(wheader(sf,(char *)&sfh)) {
	       printf("Can't seem to write header on file %s\n",sfname);
		perror("main");
		exit(1);
	}

	if (length) {      /* do this last, after wheader */
		putlength(sfname,sf,&sfh);
	}

	return 0;
}
