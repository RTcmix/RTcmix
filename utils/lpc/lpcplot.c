
/* program to display and edit lpc analysis file. It has 5 commands.
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
   3) v frame1 frame2 value.
        This will add value to the error numbers of frames frame1 toframe2.
        In this way you can force a voiced or unvoiced decision by 
        subtracting 1 or adding 1, respectively.
   4) a frame1 frame2 value
        This will multiply rms by value for frames frame1 to frame2.
        In this way you can force some amplitudes to be louder than
        others.
   5) s frame1 frame2 ntimes.
        This will smooth the pitch values of frames frame1 to frame2.
        This is accomplished by a simple 1-pole filter.  This process is
        repeated ntimes to allow any desired degree of smoothness.
   6) S frame1 frame2 ntimes.
        This will smooth the amplitude values of frames frame1 to frame2.
        This is accomplished in the same manner as the pitch smoothing.
   7) null line (blank space-return)
        this will simply display the next chunk of frames according to
        the currently defined chunk size.
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FLOAT 4
#define THRESH 2
#define AMP 1
#define RESIDAMP 0
#define PITCH 3   /* location 0 is residual rms, these are other values */
#define NUMBERS 32   /* numbers take up about 32 columns */
#define POFFSET (long)(PITCH*FLOAT)   /* byte offset for pitch value in frame*/
#define VOFFSET (long)(THRESH*FLOAT)   /* byte offset for voiced/unvoiced test*/
#define AOFFSET (long)(RESIDAMP*FLOAT)   /* byte offset for resid amp*/

int anal;
char buffer[2048];
char *bufp;
float c[50];
int endframe = 0;  /* used by getfr, reset to 0 to force reread of disk */
#ifdef USE_HEADERS
int lphs = 0;	/* header size */
#else
#define lphs (0)
#endif

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
        int j,i,xframe1,xframe2,frame1,frame2,apoints,ppoints;
        int range,diff, ntimes, firstprompt=1;
        float pbot,ptop,pmult,vval;
        int width;
        int shutup(),(*oldint)();
        float thresh,amax,amin,pmax,pmin;
        char  *output;

	if(argc == 1) {
		printf("usage:\n lpcplot lpc_data_set\n commands:\n");
		usage();
		exit();
	}

	printf("  \n"); /* seem to have to invoke printf first ???? */
        output = argv[1];

        if((anal = open(output,2)) < 0) {
                fprintf(stderr," Can't open anal file\n");
                exit(1);
        }
        fprintf(stderr," Enter npoles, thresh\t");
        scanf("%d %f",&NPOLES,&thresh);
#ifdef USE_HEADERS
        if((lphs = checkForHeader(anal, &NPOLES, 0.0)) < 0) exit(1);
#endif
	width = 80; /* hardwire 80 column output */
        range = (width - NUMBERS)/2;
        fstat(anal,&st);
        printf("Last frame is %d. Type ? for list of commands\n",(st.st_size-lphs)/((NPOLES+4)*4));
        FRAMSIZE =  (NPOLES+4);
        RECSIZE  =  (FPREC*FRAMSIZE);
        BPREC =  (RECSIZE*FLOAT);
        BPFRAME =  (FRAMSIZE*FLOAT);
        while(1) {
                while(1) {
next:           	fprintf(stderr, firstprompt ? "" : "----> ?\t");
			gets(buffer);
			firstprompt = 0;
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
                        if(*bufp == '?') {
                               	usage(); 
                                goto next;
                        }
                        if(*bufp == 'v') { /* correct thresh values
                                            * add 1 to force uv or subt 1
                                            * to force voiced */
                                sscanf(++bufp,"%d %d %f",
                                        &xframe1,&xframe2,&vval);
                                if(!xframe2) xframe2=xframe1;
                                if(!vval) {
                                        fprintf(stderr,
                                           "you didnt specify a 3rd arg\n");
                                        goto next;  
                                }
                                valter(xframe1,xframe2,vval);
                                endframe=0;
                                goto next;
                        }
                        if(*bufp == 'a') { /* correct amp values
                                             */
                                sscanf(++bufp,"%d %d %f",
                                        &xframe1,&xframe2,&vval);
                                if(!xframe2) xframe2=xframe1;
                                if(!vval) {
                                        fprintf(stderr,
                                           "you didnt specify a 3rd arg\n");
                                        goto next;  
                                }
                                aalter(xframe1,xframe2,vval);
                                endframe=0;
                                goto next;
                        }
                        if(*bufp == 's') { /* smooth pitch values */
                                sscanf(++bufp,"%d %d %d", &xframe1,&xframe2, &ntimes);
                                if(!xframe2) {
                                        fprintf(stderr,
                                           "you didnt specify the end frame.\n");
                                        goto next;  
                                }
				if(ntimes < 0) {
                                        fprintf(stderr, "ntimes must be >= 0.\n");
                                        goto next;  
				}
                                while(ntimes--) pchsmooth(xframe1,xframe2);
                                endframe=0;
                                goto next;
                        }
                        if(*bufp == 'S') { /* smooth amp values */
                                sscanf(++bufp,"%d %d %d", &xframe1,&xframe2, &ntimes);
                                if(!xframe2) {
                                        fprintf(stderr,
                                           "you didnt specify the end frame.\n");
                                        goto next;  
                                }
				if(ntimes < 0) {
                                        fprintf(stderr, "ntimes must be >= 0.\n");
                                        goto next;  
				}
                                while(ntimes--) ampsmooth(xframe1,xframe2);
                                endframe=0;
                                goto next;
                        }
                }
                pmax=amax=0;
                pmin=amin=99999999.;
                for(i=frame1;i<=frame2;i++) {
                        if(getfr(i,c) == -1) break;
                        if(c[AMP] > amax) amax = c[AMP];
                        if(c[AMP] < amin) amin = c[AMP];
                        if(c[PITCH] > pmax) pmax = c[PITCH];
                        if(c[PITCH] < pmin) pmin = c[PITCH];
                }
                ppoints = apoints = range;
                for(i=frame1;i<=frame2;i++) {
                        if(getfr(i,c) == -1) break;
                        if(pmax != pmin) 
                                ppoints = ((c[PITCH]-pmin)/(pmax-pmin)) * range;
                        if(amax != amin)
                                apoints = ((c[AMP]-amin)/(amax-amin)) * range;
                        printf("%5d ",i);
                        printf("v%0.4f p%8.3f ",c[THRESH],c[PITCH]);
                        for(j=0;j<ppoints;j++) {
                                if(c[THRESH] < thresh) putchar('*');
                                else putchar('-');
                        }
                        for(j=ppoints;j<range;j++) putchar(' ');
                        printf("a%5.0f",c[AMP]);
                        for(j=0;j<apoints;j++) {
                                if(c[THRESH] < thresh) putchar('*');
                                else putchar('-');
                        }
                        printf("\n");
                }
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
                        if((lseek(anal,lphs+((long)i*(long)BPFRAME+POFFSET),0)) 
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
                        if((lseek(anal,lphs+((long)i*(long)BPFRAME+POFFSET),0)) 
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
aalter(frame1,frame2,vval)
float vval;
{
        int i,n;
        for(i=frame1; i<=frame2; i++) {
                if(getfr(i,c) == -1) break;
                c[RESIDAMP] *= vval;
                c[AMP] *= vval;
                if((lseek(anal,lphs+((long)i*(long)BPFRAME+AOFFSET),0)) < 0) {
                                fprintf(stderr,"bad lseek\n");
                                exit(-1); 
                        }
                if((n=write(anal,(char *)(c+RESIDAMP),2 * FLOAT)) != (2 * FLOAT)) {
                        fprintf(stderr,"bad write %d\n",n);
                        exit(-1);
                }
        }
}
valter(frame1,frame2,vval)
float vval;
{
        int i,n;
        for(i=frame1; i<=frame2; i++) {
                if(getfr(i,c) == -1) break;
                c[THRESH] += vval;
                if((lseek(anal,lphs+((long)i*(long)BPFRAME+VOFFSET),0)) < 0) {
                                fprintf(stderr,"bad lseek\n");
                                exit(-1); 
                        }
                if((n=write(anal,(char *)(c+THRESH),FLOAT)) !=FLOAT) {
                        fprintf(stderr,"bad write %d\n",n);
                        exit(-1);
                }
        }
}

getfr(frame,c)
float *c;
{
        int i,j,nfr,nread;
        static float array[1000]; 
        static int oldframe = 0;
        if(!((frame >= oldframe) && (frame < endframe))) {
                if(lseek(anal,lphs+((long)frame*(long)BPFRAME),0) < 0) {
                        fprintf(stderr,"bad lseek on anal read\n");
                        return(-1); 
                }
                if((nread = read(anal,(char *)array,BPREC)) <= 0) {
                        fprintf(stderr,"bad read on anal file\n");
                        return(-1);
                }
                if(nread < BPREC) {
                        nfr = nread/BPFRAME;
                        for(i=nfr*FRAMSIZE; i<RECSIZE; i++) 
                                array[i]=0;
                        }
                oldframe = frame;
                endframe = oldframe + FPREC - 1;
        }
        for(i=(frame-oldframe)*FRAMSIZE,j=0; j<FRAMSIZE; i++,j++)  
                *(c+j) = *(array+i);
        return(0);
}

int
pchsmooth(frame1,frame2)
{
        int i,n;
        float previous;
	if(getfr((frame1),c) == -1) return;
	previous = c[PITCH];	/* pre-load filter history */
        for(i=frame1;i<=frame2;i++) {
                if(getfr(i,c) == -1) break;
		if((lseek(anal,lphs+((long)i*(long)BPFRAME+POFFSET),0)) < 0) {
			fprintf(stderr,"bad lseek\n");
			exit(-1); 
		}
		c[PITCH] = (0.5 * previous) + (0.5 * c[PITCH]); /* filter */
		previous = c[PITCH];
		if((n=write(anal,(char *)(c+PITCH),FLOAT)) != FLOAT) {
			fprintf(stderr,"bad write %d\n",n);
			exit(-1);
		}
        }
}

int
ampsmooth(frame1,frame2)
{
        int i,n, bytes = 2 * FLOAT;
        float aprevious, rprevious;
	if(getfr((frame1),c) == -1) return;
	aprevious = c[AMP];	/* pre-load filter histories */
	rprevious = c[RESIDAMP];
        for(i=frame1;i<=frame2;i++) {
                if(getfr(i,c) == -1) break;
		if((lseek(anal,lphs+((long)i*(long)BPFRAME+AOFFSET),0)) < 0) {
			fprintf(stderr,"bad lseek\n");
			exit(-1); 
		}
		c[AMP] = (0.5 * aprevious) + (0.5 * c[AMP]); /* filter */
		c[RESIDAMP] = (0.5 * rprevious) + (0.5 * c[RESIDAMP]);
		aprevious = c[AMP];
		rprevious = c[RESIDAMP];
		if((n=write(anal,(char *)(c+RESIDAMP), bytes)) != bytes) {
			fprintf(stderr,"bad write %d\n",n);
			exit(-1);
		}
        }
}

usage()
{
printf("\n\
   lpcplot commands:\n\
   1) n frame1 frame2\n\
        will display frame1 to frame2.  If frame2 is 0 it will begin\n\
        current chunk size on frame1.  If frame1 is negative it will\n\
        backup that many frames and display current chunk size from \n\
        there.  If neither frame1 or frame2 is specified it will\n\
        simply display previous segment again.\n\
   2) p frame1 frame2 mult botcps topcps\n\
        Multiply the pitches of frames frame1 to frame2 by mult if the\n\
        current pitch is >= botcps and <=topcps.  default for botcps\n\
        and top cps are 0 and 999999.  If mult is 0 it will interpolate\n\
        pitches in these frames according to the frequencies of frame\n\
        frame1-1 and frame2+1.  If frame2 is 0 it defaults to frame1.\n\
        Thus p 100 will simply replace the frequency of frame100 by \n\
        interpolating between frames 99 and 101.\n\
   3) v frame1 frame2 value.\n\
        This will add value to the error numbers of frames frame1 to frame2.\n\
        In this way you can force a voiced or unvoiced decision by \n\
        subtracting 1 or adding 1, respectively.\n\
   4) s frame1 frame2 ntimes.\n\
        This will smooth the pitch values of frames frame1 to frame2.\n\
        This is accomplished by a simple 1-pole filter.  This process is \n\
        repeated ntimes to allow any desired degree of smoothness.\n\
   5) S frame1 frame2 ntimes.\n\
        This will smooth the amplitude values of frames frame1 to frame2.\n\
        This is accomplished in the same manner as the pitch smoothing.\n\
   6) null line (blank space-return)\n\
        this will simply display the next chunk of frames according to\n\
        the currently defined chunk size.\n");
}
