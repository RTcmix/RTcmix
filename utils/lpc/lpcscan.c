#include "../../H/ugens.h"
#include <stdio.h>
#include <sys/stat.h>
#define RESIDAMP     0
#define RMSAMP       1
#define THRESH       2
#define PITCH	     3  /* values in locs 0-3 of frame*/

#define MAXPOLES 50
#define MAXPITCHES 24000
int framsize;
int fprec;
int recsize;
int bprec;
int bpframe;
int npolem1;
int anal;
int i;
float cq,outold;
float FloatJunk;
char dataset_name[80];
float thresh,randamp,randoff,unvoiced_rate;
int npoles=0;
#ifdef USE_HEADERS
int lphs;       /* header size */
#else
#define lphs (0)
#endif


float the_top = 100000.0; /* top number for scanning */

float *ballpole(),alpvals[2048],buzvals[2048],pchval[MAXPITCHES];

double
lpcscan(p,n_args)
float *p;
{
	float amp,si,hn,phs,*f,srd2,magic,d,warpset();
	float c[MAXPOLES+4],past[MAXPOLES*2],frames,frameno,ampmlt,errno;
	int frame1;
	float *cpoint;
	float x,transposition,newpch;
	int jcount,seed,i,nsamps,counter;
	int nn,lastfr;
	float transp,lasttr,tranincr,frameseg;
	float cps,tblvals[2],weight();

	if(!n_args) {
		printf(" p[0]=starting time, p[1]=duration, p[2]=pitch,p[3]=frame1, p4=frame2 p[5]=amp, p[6]=warp\n");
		return;
	}
	sbrrand(.1);
	for(i=0; i<npoles*2; i++) past[i] = 0;

	if(anal <=0 ) {
		printf("%d !!!No open dataset!",anal);
		closesf();
		}
	srd2 = SR/2.;
	magic = 512./SR;
	jcount = phs = counter = 0; 
	seed = .1;
	cpoint = c + 4;
	nsamps = setnote(p[0],p[1],0);
	frames = p[4] - p[3] + 1.;
	if(frames >= MAXPITCHES) {
		printf("pchval array is insufficiently dimensioned for this");
		printf("rather large (%f) set of frames.\n",frames);
		exit(-1);
		}
	frame1 = (int)p[3];
	for(i=p[3]; i<= p[4]; i++) {
		d = i;
		getfr(d,c);
		if(c[PITCH]==0) {
			printf("There is a hole in the pitch analysis.\n");
			printf("It occurs at frame %d.\n",i);
			exit(-1);
			}
		pchval[i - (int)p[3]] = c[PITCH];
	}
		if(ABS(p[2]) < 1.)  {
			FloatJunk=(p[2]/.12);
			FloatJunk=pow(2.0,FloatJunk);
			transposition=FloatJunk;
		}
		else	transposition = cpspch(p[2])/weight(cpspch(p[2]),p[3],p[4],thresh);
	if(n_args == 7) {		
		printf("Overall transposition: %f\n",transposition);
		for(i=frame1;i<=(int)p[4];i++) {
			pchval[(i - frame1)] *= transposition;
			}
		}
	else {
/*   old code --------------- */
/*
		lastfr=frame1;
		lasttr=transposition;
		while((nn+=2)<n_args) {
			if(ABS(p[nn+1]) < 1.) transp=pow(2.0,(p[nn+1]/.12)); 
			else {
				transp = cpspch(ABS(p[nn+1]))/weight(cpspch(ABS(p[nn+1])),(float)lastfr,(p[nn]+1.),thresh);
				}
				tranincr=(transp-lasttr)/(p[nn]-lastfr);
				transp=lasttr;
				for(i=lastfr;i<(int)p[nn];i++) {
					pchval[i-frame1]*=transp;
					transp+=tranincr;
					}	
				lastfr=p[nn];
				lasttr=transp;
				}
			if(p[n_args-2] < p[4]) {
*/
		/* last frame in couplets wasn't last frame in batch, so use
				base value from there to the end */
/*
				transp = transposition;
				for(i=lastfr;i<(int)p[4];i++) {
					pchval[i-frame1]*=transp;
					}
				}
*/
/*   end of old code -------------- */
		nn = 5;
		while((nn+=2)<n_args) {
			frameseg = p[nn+2] - p[nn];
			tranincr=((cpspch(p[nn+3]))-(cpspch(p[nn+1])))/frameseg;
			transp = cpspch(p[nn+1]);
			for (i = p[nn]; i < p[nn+2]; i++) {
				pchval[i - frame1] = transp;
				transp += tranincr;
				}
			}
		}


	tableset(p[1],(int)(p[4]-p[3]+1),tblvals);
	amp = p[5];
	d = p[6];
	f = (float *)floc(1);


	warpinit();
	for(i=nsamps; i>0; i -=counter) {
			frameno = ((float)(nsamps - i)/nsamps)*frames + frame1;
			if(getfr(frameno,c) == -1) break;
			errno = (c[2] > thresh) ? 0 : 1;  
			ampmlt = (errno) ? amp * c[0] : amp * c[0] * randamp;
			cps = tablei(nsamps-i,pchval,tblvals);
			newpch = cps;
			si = newpch * magic;
			hn = (int)(srd2/newpch);
			counter = (float)(SR/newpch);
			counter = (counter > i) ? i : counter;
			if(counter <= 0) break;

			if(errno) 
				bbuzz(ampmlt,si,hn,f,&phs,buzvals,counter);
			else
				brrand(ampmlt,buzvals,counter);
			if(d) {
                                ampmlt *=  warpset(d,cpoint);
				bwarppol(buzvals,past,d,cpoint,alpvals,counter);
			}
			else
				{
				ballpole(buzvals,&jcount,npoles,
		               			past,cpoint,alpvals,counter);
				}

			if ( bcheck(alpvals,counter,frameno) < 0 )
				exit(-1);
	}
/*	endnote(0);	not needed, is it?  */
	fprintf(stderr, "no bad frames found.\n");
	return 0;	
}

float warppol(sig,past,d,c)
float sig,*past,d,*c;
{
	float temp1,temp2;
	int n;

	temp1 = past[npolem1];
	past[npolem1] = cq * outold - d * past[npolem1];
	for(n=npoles-2; n>=0; n--) {
		temp2 = past[n];
		past[n] = d * (past[n+1] - past[n]) + temp1;
		temp1 = temp2;
		}
	for(n=0;n<npoles;n++)  sig += c[n] * past[n];
	outold = sig;
	return(sig);
}
float warpset(d,c)     
float d,*c;
{
	int m;
	float cl;

	for(m=1; m<npoles; m++)   c[m] += d * c[m-1];
	cl = 1./(1.-d * c[npolem1]);
	cq = cl * (1. - d * d);
	return(cl);
}
warpinit()
{
	outold = 0;
}
bwarppol(sig,past,d,c,out,nvals)
float *sig,*past,d,*c,*out;
{
	float temp1,temp2;
	int i,n;
	for(i=0; i<nvals; i++) {
		temp1 = past[npolem1];
		past[npolem1] = cq * outold - d * past[npolem1];
		for(n=npoles-2; n>=0; n--) {
			temp2 = past[n];
			past[n] = d * (past[n+1] - past[n]) + temp1;
			temp1 = temp2;
			}
		for(n=0;n<npoles;n++)  *sig += c[n] * past[n];
		*out++ = outold = *sig++;
	}
}
getfr(frameno,c)
float frameno,*c;
{
	int frame,i,j;
	static float array[22*(MAXPOLES+4)];
	float fraction;
	static int oldframe = 0;
	static int endframe = 0;
	frame = (int)frameno;
	fraction = frameno - (float)frame;
	if(!((frame >= oldframe) && (frame < endframe))) {
		if(lseek(anal,lphs+((long)frame*(long)bpframe),0) == -1) {
			fprintf(stderr,"bad lseek on analysis file \n");
			return(-1);
		} 
		if(read(anal,(char *)array,bprec) <= 0) {
			fprintf(stderr,"reached eof on analysis file \n");
			return(-1);
		}
		oldframe = frame;
		endframe = oldframe + fprec - 1;
		}

	for(i=(frame-oldframe)*framsize,j=0; j<framsize; i++,j++)  
		*(c+j) = *(array+i) + fraction * (*(array+i+framsize) 
						- *(array+i));
	return(0);
}
static	long	randx = 1;

/*
srrand(x)
unsigned x;
{
	randx = x;
}
*/

float weight(newpch,frame1,frame2,throsh)
float newpch,frame1,frame2;
{
	float c[MAXPOLES+4];
	int i;
	float xweight,sum;
	xweight = sum = 0;
	for(i=(int)frame1; i<(int)frame2; i++) {
		getfr((float)i,c);
		if(c[THRESH] <= throsh) {
			xweight += c[RMSAMP];
			sum += (c[PITCH] * c[RMSAMP]);
		}
	}
	return(sum/xweight);
}

double
dataset(p,n_args)
/* p1=dataset name, p2=npoles */
float *p;
int n_args;
{
        char *name;
        int i;
	struct stat st;
        fprec=22;
        if(n_args>1)	/* if no npoles specified, it will be retrieved from */
                npoles=p[1];	/* the header (if USE_HEADERS #defined) */
        lpcinit();
        i=(int)p[0];
        name=(char *)i;
        if(strcmp(name,dataset_name)== 0) {
                printf("\n%s is already open.\n",name);
                return;
        }
        strcpy(dataset_name,name);
        if((anal = open(name,0)) <= 0) {
                printf("Can't open %s\n",name);
                closesf();
        }
	printf("\nOpened dataset %s.\n",name);
#ifdef USE_HEADERS
	if((lphs = checkForHeader(anal, &npoles, SR)) < 0) closesf();
#else
	if(!npoles) {
		fprintf(stderr,
			"You must specify the correct value for npoles in p[1].\n");
		closesf();
	}
#endif /* USE_HEADERS */

	npolem1=npoles-1;
	framsize=npoles+4;
	recsize=fprec*framsize;
	bprec=recsize*FLOAT;
	bpframe=framsize*FLOAT;

	/* return number of frames in datafile */
	if(stat(name, &st) >= 0) {
		int frms = (st.st_size-lphs) / ((npoles+4) * sizeof(float));
		printf("File has %d frames.\n", frms);
		return (float) frms;
	}
	else {
		fprintf(stderr, "Unable to stat dataset file.\n");
		closesf();
		return 0.0;	/* not reached */
	}
}

double
lpcstuff(p,n_args)
/* p1=thresh, p2=random amp, p3=unvoiced rate */
float *p;
int n_args;
{
	if(n_args>0)
		thresh=p[0];
	if(n_args>1)
		randamp=p[1];
	if(n_args>2)
		unvoiced_rate=p[2];
	printf("\nAdjusting settings for %s.\n",dataset_name);
	printf(" --------------------------------------- \n");
	printf("Thresh: %f     Randamp: %f\n",thresh,randamp);
	if(unvoiced_rate == 1)
		printf("Unvoiced frames played at normal rate.\n\n");
	else
		printf("Unvoiced frames played at same rate as voiced 'uns.\n\n");
}

lpcinit()
{
recsize=framsize*fprec;
bprec=recsize*FLOAT;
bpframe=framsize*FLOAT;
}

/* block version of rrand */
/* a modification of unix rand() to return floating point values between
   + and - 1. */

/*
sbrrand(x)
unsigned x;
{
	randx = x;
}
brrand(am,a,j)
float am,*a;
{
	int k;
	for(k=0; k<j; k++) {
		int i = ((randx = randx*1103515245 + 12345)>>16) & 077777;
		*a++ = (float)(i/16384. - 1.) * am;
	}
}
*/

bcheck(arr,nvals,fr)

float *arr,fr;
int nvals;
{
	int i;
	extern float the_top;

	for (i = 0; i < nvals; i++) {
		if (arr[i] > the_top) {
			fprintf(stderr,"exploding value at frame %d (%f)\n",(int)fr,fr);
			return(-1);
			}
		}
	return(1);
}


double
topnumber(p,n_args)
float *p;
int n_args;

{
	extern float the_top;

	the_top = p[0];
	fprintf(stderr,"scan peak set at: %f\n",the_top);
	return(1);
}

int NBYTES = 32768; 

profile()
{
	float p[9];
	UG_INTRO("lpcscan",lpcscan);
	UG_INTRO("lpcstuff",lpcstuff);
	UG_INTRO("dataset",dataset);
	UG_INTRO("topnumber", topnumber);
	p[0]=1; p[1]=10;
	p[2]=1024; p[3]=1;
	makegen(p,4); /* store sinewave in array 1 */
}
