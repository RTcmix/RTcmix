/* program to display and edit pch analysis file. It has 3 commands.
   1) n frame1 frame2
	will display frame1 to frame2.  If frame2 is 0 it will begin
	current chunk size on frame1.  If frame1 is negative it will
	backup that many frames and display current chunk size from 
	there.  If neither frame1 or frame2 is specified it will
	simply display previous segment again.
   2) p frame1 frame2 mult botcps topcps
	Multiply the pitches of frames frame1 to frame2 by mult if the
	current pitch is >= botcps and <=topcps.  default for botcps
	and top cps are 0 and 999999.  If mult is 0 it will interpolate
	pitches in these frames according to the frequencies of frame
	frame1-1 and frame2+1.  If frame2 is 0 it defaults to frame1.
	Thus p 100 will simply replace the frequency of frame100 by 
	interpolating between frames 99 and 101.
   3) null line (blank space-return)
	this will simply display the next chunk of frames according to
	the currently defined chunk size.
*/
#include <stdio.h>
#include <signal.h>
#include "../../H/ugens.h"
#include <sys/types.h>
#include <sys/stat.h>

#define THRESH 2
#define AMP 1
#define PITCH 0   
#define NUMBERS 32   /* numbers take up about 32 columns */
#define POFFSET (long)(PITCH*FLOAT)   /* byte offset for pitch value in frame*/
#define VOFFSET (long)(THRESH*FLOAT)   /* byte offset for voiced/unvoiced test*/

int anal;
int busy;
char buffer[4096];
char *bufp;
float c[50];
int endframe = 0;  /* used by getfr, reset to 0 to force reread of disk */

int NPOLES;
int FRAMSIZE;    
int FPREC = 1;
int RECSIZE;  
int BPREC;   
int BPFRAME;

main(argc,argv)

int argc;
char *argv[];
{
	struct stat st;
	int j,i,args,xframe1,xframe2,frame1,frame2,apoints,ppoints,medianrange;
	int range,diff;
	float pbot,ptop,pmult,vval;
	int scaler,width;
	void shutup();
	float thresh,amax,amin,pmax,pmin;
	char  *output;
	if(argc == 1) {
		printf("usage:\n pchplot pitch_analysis_file\n commands:\n");
		usage();
		exit();
	}

	 output = argv[1];

	if((anal = open(output,2)) < 0) {
		fprintf(stderr," Can't open anal file\n");
		exit(1);
	}
	fstat(anal,&st);
        printf("Last frame is %d. Type ? for list of commands\n",(int)(st.st_size/8-1));
/*
	//printf("%d %d %d\n",st.st_size,st.st_blksize,st.st_blocks);

//	fprintf(stderr," Enter width of plot\t");
//	scanf("%d",&width);
//	fprintf(stderr," Scale output per segment y/n?\t");
//	scanf("%s",output);
//	if(*output == 'y') scaler = 1;
//	else scaler = 0;
*/
	width = 80;
	scaler = 1;
	range = (width - NUMBERS)/2;
	FRAMSIZE =  2;
	RECSIZE  =  (FPREC*FRAMSIZE);
	BPREC =  (RECSIZE*FLOAT);
	BPFRAME =  (FRAMSIZE*FLOAT);
	while(1) {
		while(1) {
next:       		fprintf(stderr,"----> ?\t");
			gets(buffer);
			bufp = buffer;
			if(*bufp == 'n') {
				xframe1=frame1;
				xframe2=frame2;
				frame2 = 0;
				sscanf(++bufp,"%d %d",&frame1,&frame2);
				if(frame1 < 0) {/*back up but keep same range*/
					frame1 += xframe2;
					frame2 = (xframe2 - xframe1)+frame1;
				}
				if(!frame2) frame2 = (xframe2-xframe1)+frame1;
				break;
			}
			if(*bufp == ' ') {
				diff=frame2-frame1;
				frame1=frame2+1;
				frame2=frame1+diff;
				break;
			}
			if(*bufp == 'p') {  /* correct pitch values */
				xframe1=xframe2=pbot=pmult=0;
				ptop = 999999.;
				sscanf(++bufp,"%d %d %f %f %f",
				   &xframe1,&xframe2,&pmult,&pbot,&ptop);
				if(!xframe2) xframe2=xframe1;
				pchalter(xframe1,xframe2,pbot,ptop,pmult);
				endframe = 0;
				goto next;
			}
			if(*bufp == 'm') { /* median filter */
				args=sscanf(++bufp,"%d %d %d",&xframe1,&xframe2,&medianrange);
				if(args != 3) { 
					printf("not enough args\n");
					goto next;
					}
				if(!(medianrange % 2)) medianrange++;
				/* must be odd */
				median(xframe1,xframe2,medianrange);
				goto next;
			}
			if(*bufp == '?') {
				usage();
				goto next;
				}
		}
		signal(SIGINT,(void *)shutup);
		pmax=amax=0;
		pmin=amin=99999999.;
		if(scaler) {
		for(i=frame1;i<=frame2;i++) {
			if(getfr(i,c) == -1) break;
			if(c[AMP] > amax) amax = c[AMP];
			if(c[AMP] < amin) amin = c[AMP];
			if(c[PITCH] > pmax) pmax = c[PITCH];
			if(c[PITCH] < pmin) pmin = c[PITCH];
		}
		}
		else {
			amax = 32767;
			pmax = 400;
			amin = 200;
			pmin = 75;
		}
		busy=1;
		ppoints = apoints = range;
		for(i=frame1;i<=frame2;i++) {
			if(!busy) break;
			if(getfr(i,c) == -1) break;
			if(pmax != pmin) 
				ppoints = ((c[PITCH]-pmin)/(pmax-pmin)) * range;
			if(amax != amin)
				apoints = ((c[AMP]-amin)/(amax-amin)) * range;
			printf("%5d ",i);
			printf("p%8.3f ",(c[PITCH]));
			for(j=0;j<ppoints;j++) putchar('*');
			for(j=ppoints;j<range;j++) putchar(' ');
			printf("a%5.0f",c[AMP]);
			for(j=0;j<apoints;j++)  putchar('*');
			printf("\n");
		}
		fflush(stdout);
		signal(SIGINT,SIG_DFL);
	}
}
pchalter(frame1,frame2,pbot,ptop,pmult)
float pbot,ptop,pmult;
{
	int i,n;
	float begin,end,nframes;
	if(!pmult) {
		if(getfr((frame1-1),c) == -1) return;
		begin = c[PITCH];
		if(getfr((frame2+1),c) == -1) return;
		end = c[PITCH];
		nframes = 2. + frame2-frame1;
	}
	for(i=frame1;i<=frame2;i++) {
		if(getfr(i,c) == -1) break;
		if(!pmult) {
			c[PITCH] = begin + ((float)(i-frame1+1)/nframes) 
				* (end-begin);
			if((lseek(anal,((long)i*(long)BPFRAME+POFFSET),0)) 
				< 0) {
					fprintf(stderr,"bad lseek\n");
					exit(-1); 
				}
			if((n=write(anal,(char *)(c+PITCH),FLOAT)) !=FLOAT) {
				fprintf(stderr,"bad write %d\n",n);
				exit(-1);
			}
		}
		else
		if(c[PITCH] >=  pbot && c[PITCH] <= ptop) {
			if((lseek(anal,((long)i*(long)BPFRAME+POFFSET),0)) 
				< 0) {
					fprintf(stderr,"bad lseek\n");
					exit(-1); 
				}
			c[PITCH] *= pmult;
			if((n=write(anal,(char *)(c+PITCH),FLOAT)) !=FLOAT) {
				fprintf(stderr,"bad write %d\n",n);
				exit(-1);
			}
		}
	}
}
median(frame1,frame2,range)
{
	int n,i;
	float c[2],medianval();
	for(i=frame1; i<=frame2; i++) {
		if((lseek(anal,((long)i*(long)BPFRAME+POFFSET),0)) < 0) {
					fprintf(stderr,"bad lseek\n");
					exit(-1); 
				}
		c[PITCH] = medianval(i,range);
		if((n=write(anal,(char *)(c+PITCH),FLOAT)) !=FLOAT) {
				fprintf(stderr,"bad write %d\n",n);
				exit(-1);
			}
		}
}
float medianval(frameno,range)
{
	float buffer[51],c[2],min,max; int i,j,maxno,minno,median;
	for(j=0,i=frameno-(range/2); i<=frameno+(range/2); i++,j++) {
		getfr(i,c);
		buffer[j] = c[PITCH];
		}
	for(i=0,min=1e9,max=-1e9; i<(range/2); i++) {
		for(j=0; j<range; j++) {
			if(buffer[j] >= max)  max=buffer[j];
			if(buffer[j] <= min)  min=buffer[j];
		}
		for(j=0; j<range; j++) {
			if(buffer[j] == max) buffer[j]=min;
			}
		}
	for(i=0,max=-1e6; i<range; i++) { 
		if(buffer[i] > max) max=buffer[i];
		}
	return(max);
}
		


getfr(frame,c)
float *c;
{
	int i,j;
	static float array[1000]; 
	static int oldframe = 0;
	if(!((frame >= oldframe) && (frame < endframe))) {
		if(lseek(anal,((long)frame*(long)BPFRAME),0) < 0) {
			fprintf(stderr,"bad lseek on anal read\n");
			return(-1); 
		}
		if(read(anal,(char *)array,BPREC) != BPREC) {
			fprintf(stderr,"bad read on anal file\n");
			return(-1);
		}
		oldframe = frame;
		endframe = oldframe + FPREC - 1;
	}
	for(i=(frame-oldframe)*FRAMSIZE,j=0; j<FRAMSIZE; i++,j++)  
		*(c+j) = *(array+i);
	return(0);
}
void shutup()
{busy=0;}
usage()
{
printf("\n\
  1) n frame1 frame2\n\
        will display frame1 to frame2.  If frame2 is 0 it will begin\n\
        current chunk size on frame1.  If frame1 is negative it will\n\
        backup that many frames and display current chunk size from\n\
        there.  If neither frame1 or frame2 is specified it will\n\
        simply display previous segment again.\n\
   2) p frame1 frame2 mult botcps topcps\n\
        Multiply the pitches of frames frame1 to frame2 by mult if the\n\
        current pitch is >= botcps and <=topcps.  default for botcps\n\
        and top cps are 0 and 999999.  If mult is 0 it will interpolate\n\
        pitches in these frames according to the frequencies of frame\n\
        frame1-1 and frame2+1.  If frame2 is 0 it defaults to frame1.\n\
        Thus p 100 will simply replace the frequency of frame100 by\n\
        interpolating between frames 99 and 101.\n\
   3) null line (blank space-return)\n\
        this will simply display the next chunk of frames according to\n\
        the currently defined chunk size.\n");
}
