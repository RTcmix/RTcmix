#include "../../H/ugens.h"
#include "../../H/sfheader.h"
#include <math.h>
#include <stdio.h>
#include <sys/stat.h>
#define RESIDAMP     0
#define RMSAMP       1
#define THRESH       2
#define PITCH        3

int anal;
float cq,outold;
extern SFHEADER sfdesc[NFILES];

#define MAXVALS 10000
#define MAXPOLES 64
int framsize;
int fprec;
int recsize;
int bprec;
int bpframe;
int npolem1;
int anal;
int i;
float maxdev;
float perperiod = 1.0;
float FloatJunk;
float cutoff;
extern SFHEADER sfdesc[NFILES];
char dataset_name[80];
float thresh,randamp,randoff,unvoiced_rate;
int npoles=0;
float risetime,decaytime;
#ifdef USE_HEADERS
int lphs;	/* header size */
#else
#define lphs (0)
#endif

double lpcplay(p,n_args)
float *p;
{
	float amp,si,hn,phs,*f,srd2,magic,d,warpset();
	float c[MAXPOLES+4],past[MAXPOLES*2],frames,frameno,ampmlt,errnum=1;
	int frame1;
	float alpvals[MAXVALS],buzvals[MAXVALS],noisvals[MAXVALS],pchval[MAXVALS];
	float *cpoint, *bvp, *nvp, buzamp, noisamp;
	float x,transposition,newpch;
	double shift(), getVoicedAmp();
	int jcount,seed,i,nsamps,counter;
	int nn,lastfr;
	float transp,lasttr,tranincr;
	float cps,tblvals[2],weight();
	float rsnetc[9];
	int reson_is_on;
	float cf_fact,bw_fact;
	float evals[5];
	float *f1;
	float actualcps,actualweight;
	float peak;
	int jj,datafields,unit;

	if(!n_args) {
			printf(" p[0]=starting time, p[1]=duration, p[2]=pitch,p[3]=frame1, p4=frame2 p[5]=amp, p[6]=warp p7=resonbw, p8=resoncf p9--> pitchcurves\n");
			return;
	}

	unit = 1;  /* outputfile */
	f1 = floc(2);
	sbrrand(1);
	for(i=0; i<npoles*2; i++) past[i] = 0;

	if(anal <=0 ) {
			printf("%d !!!No open dataset!\n",anal);
			closesf();
	}
	srd2 = SR/2.;
	magic = 512./SR;
	jcount = phs = counter = 0; 
	datafields = 10; /* number of fields before pitch curves */
	seed = .1;
	cpoint = c + 4;
	p[1] = (p[1] > 0.) ? p[1] : ((p[4] - p[3] + 1.)/112.);
	evset(p[1],risetime,decaytime,2,evals);
	nsamps = setnote(p[0],p[1],unit);
	frames = p[4] - p[3] + 1.;
	frame1 = (int)p[3];
	for(i=p[3]; i<= p[4]; i++) {
		d = i;
		if(getfr(d,c) < 0) break;
		pchval[i - (int)p[3]] = (c[PITCH] ? c[PITCH] : 256.);
		/* just in case I am using datasets with no pitch value
			stored */
	}
	/*p[4] = i; */
	actualweight = weight(p[3],p[4],thresh);
	if(!actualweight) actualweight = cpspch(p[2]);
	if(ABS(p[2]) < 1.)
		transposition = (pow(2.0,(p[2]/.12)));
	else if(p[2] > 0)
		transposition = cpspch(p[2])/actualweight;
	else
		transposition = cpspch(-p[2]);  /* flat pitch */
	if((n_args <= datafields) && (p[2] > 0)) {
		printf("\nOverall transposition: %f, weighted av. pitch = %f\n",
			transposition,actualweight);
		if(maxdev) 
			readjust(maxdev,pchval,p[3],p[4],thresh,actualweight);
		for(i=frame1;i<=(int)p[4];i++) {
			pchval[(i - frame1)] *= transposition;
		}
	}
	else {
		nn=datafields-1;
		lastfr=frame1;
		lasttr=transposition;
		while((nn+=2)<n_args) {
			if(ABS(p[nn+1]) < 1.) {
				transp = pow(2.0,(p[nn+1]/.12));
			}
			else {
				transp = cpspch(ABS(p[nn+1])) / 
					weight((float)lastfr,(p[nn]+1.),thresh);
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
			/* if last frame in couplets wasn't last frame in batch,
			   use base value from there to the end */
			transp = transposition;
			for(i=lastfr;i<(int)p[4];i++)
				pchval[i-frame1]*=transp;
		}
	}
	tableset(p[1],(int)(p[4]-p[3]+1),tblvals);
	amp = p[5];
	d = p[6];
	actualweight = weight(p[3],p[4],thresh);
	actualcps = cpspch(ABS(p[2]));
	
	/* d = (d>1.) ? shift(actualweight,actualcps,(float)SR) : p[6]; */
	d = (d>1.) ? .0001 : p[6];
	
	/* note, dont use this feature unless pitch is specified in p[2]*/
	
	f = (float *)floc(1);
	for(i=0; i<9; i++) rsnetc[i]=0;
	reson_is_on = p[7] ? 1 : 0;
	cf_fact = p[7];
	bw_fact = p[8];
	frameno = frame1;	/* in case first frame is unvoiced */
	
	warpinit();
	for(i=nsamps; i>0; i -=counter) {
		int loc;
		if ( unvoiced_rate && !errnum )
			frameno++;	/* if unvoiced set to normal rate */
		else {
			frameno = ((float)(nsamps - i)/nsamps)*frames + frame1;
		}
		if(getfr(frameno,c) == -1) goto out;
		buzamp = getVoicedAmp(c[THRESH]);
		errnum = (buzamp != 0.0); /* errnum = 0 for total noise only*/
		noisamp = (1.0 - buzamp) * randamp;	/* for now */
		ampmlt = amp * c[RESIDAMP];
		if(c[RMSAMP] < cutoff) ampmlt = 0;
		cps = tablei(nsamps-i,pchval,tblvals);
		newpch = cps;
		if((p[2]< 0) &&  (ABS(p[2]) >= 1)) newpch = transposition;
		if(reson_is_on) {
			rszset(cf_fact*cps, bw_fact*cf_fact*cps,1.,rsnetc);
			/* printf("%f %f %f %f\n",cf_fact*cps,
				bw_fact*cf_fact*cps,cf_fact,bw_fact,cps); */
		}
		si = newpch * magic;
		ampmlt *= evp(nsamps-i,f1,f1,evals);

		if(d)
			ampmlt *=  warpset(d,cpoint);
		hn = (int)(srd2/newpch)-2;
		counter = (float)(SR/(newpch * perperiod) ) * .5;
		counter = (counter > i) ? i : counter;
#ifdef debug
		printf("f %f e %f b %f n %f p %f c %d\n",
		 frameno,errnum,ampmlt*buzamp,ampmlt*noisamp,newpch,counter);
#endif
                if(counter <= 0) break;

		bbuzz(ampmlt*buzamp,si,hn,f,&phs,buzvals,counter);
		brrand(ampmlt*noisamp,noisvals,counter);
		bvp = buzvals; nvp = noisvals;
		for(loc=0; loc<counter; loc++)
			*bvp++ += *nvp++;	/* add voiced and unvoiced */
			if(d) {
				d = (p[6] > 1.) ? shift(c[3],newpch,(float)SR) : p[6];
				/* printf("d=%f %f %f\n",c[3],newpch,d);        */
				/*************
				d = ABS(d) > .2 ? SIGN(d) * .15 : d;
				***************/
				bwarppol(buzvals,past,d,cpoint,alpvals,counter);
			}
			else
			{
				ballpole(buzvals,&jcount,npoles,past,cpoint,alpvals,counter);
			}
			if(reson_is_on)
				bresonz(alpvals,rsnetc,alpvals,counter);
			baddout(alpvals,unit,counter);
        }
out:    endnote(unit);
}

double lpcin(p,n_args)
float *p;
{
	float amp,si,hn,phs,*f,srd2,magic,d,warpset();
	float c[MAXPOLES+4],past[MAXPOLES*2],frames,frame1,frameno,ampmlt,errnum;
	float alpvals[2048],buzvals[2048];
	float randamp,randoff;
	float *cpoint;
	float x,transposition,newpch;
	int jcount,seed,i,nsamps,counter;
	float tblvals[2];
	int nread,input, output, j,k;


	if(n_args != 7) {
printf(" incorrect argument list: p[0]=starting time, p[1]=duration,p[2]=frame1, p[3]=frame2 p[4]=amp,p[5]=d,p[6]=inputskip\n");
		return;
	}
	if(anal <= 0) { printf("No lpc dataset has been opened\n"); return;}
	input = 0; output=1;
	if(sfchans(&sfdesc[output]) != 1) { 
		printf("Output file must have 1 channel only\n");
		return;
		}
	if(sfchans(&sfdesc[input]) != 1) { 
		printf("Input file must have 1 channel only\n");
		return;
		}
	
	for(i=0; i<npoles*2; i++) past[i] = 0;

	srd2 = SR/2.;
	magic = 512./SR;
	jcount = phs = counter = 0; 
	randamp = .1;
	seed = .1;
	cpoint = c + 4;

	nsamps = setnote(p[0],p[1],output);
                 setnote(p[6],p[1],input);
	frames = p[3] - p[2] + 1.;
	frame1 = p[2];
	amp = p[4];
	d = p[5];

	if(anal <=0 ) {
		printf("No open dataset!\n");
		return;	
	}

	warpinit();
	for(i=nsamps; i>0; i -=counter) {
		frameno = ((float)(nsamps - i)/nsamps)*frames + frame1;
		if(getfr(frameno,c) == -1) break;
		ampmlt = c[0];
		newpch = (c[3] > 0.0) ? c[3] : 256.0;
		counter = (float)(SR/newpch);
		counter = (counter > i) ? i : counter;
		if(counter <= 0) break;

		nread=bgetin(buzvals,input,counter);
		if(!nread)  break;
		if(d) {
			ampmlt *=  warpset(d,cpoint);
			bwarppol(buzvals,past,d,cpoint,alpvals,counter);
		}
		else
			ballpole(buzvals,&jcount,npoles,past,cpoint,alpvals,counter);
		bmultf(alpvals,ampmlt,counter);
		bwipeout(alpvals,output,counter); 
	}
	endnote(output);
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

        for(m=1; m<npoles; m++)
			c[m] += d * c[m-1];
        cl = 1./(1. - d * c[npolem1]);
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
static  long    randx = 1;

/*
srrand(x)
unsigned x;
{
        randx = x;
}
*/

float weight(frame1,frame2,throsh)
float frame1,frame2,throsh;
{
        float c[MAXPOLES+4];
        int i;
        float xweight,sum;
        xweight = sum = .001;
        for(i=(int)frame1; i<(int)frame2; i++) {
                getfr((float)i,c);
                if((c[THRESH] <= throsh) || (throsh < 0.)) {
                        xweight += c[RMSAMP];
                        sum += (c[PITCH] * c[RMSAMP]);
                }
        }
        return(sum/xweight);
}

double dataset(p,n_args,pp)
/* p1=dataset name, p2=npoles */
float *p;
int n_args;
double *pp;
{
	char *name;
	struct stat st;
	fprec=22;
	if(n_args>1)	/* if no npoles specified, it will be retrieved from */
			npoles=p[1];	/* the header (if USE_HEADERS #defined) */
	lpcinit();
	name = DOUBLE_TO_STRING(pp[0]);
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

double lpcstuff(p,n_args)
/* p0=thresh, p1=random amp, p2=unvoiced rate p3= rise, p4= dec, p5=thresh cutof*/
float *p;
int n_args;
{
        risetime=.01; decaytime=.1;
        if(n_args>0) thresh=p[0];
        if(n_args>1) randamp=p[1];
        if(n_args>2) unvoiced_rate=p[2];
        if(n_args>3) risetime=p[3];
        if(n_args>4) decaytime=p[4];
        if(n_args>5) cutoff = p[5]; else cutoff = 0;
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
double freset(p,n_args)
float *p;
{
        perperiod = p[0];
        printf("Frame reinitialization reset to %f times per period.\n",perperiod);
}

double lowthresh, highthresh;
int NBYTES = 32768;

profile()
{
	float p[9];
	UG_INTRO("lpcplay",lpcplay);
	UG_INTRO("lpcstuff",lpcstuff);
	UG_INTRO("dataset",dataset);
	UG_INTRO("freset",freset);
	UG_INTRO("mp",mp);
	UG_INTRO("mpset",mpset);
	UG_INTRO("setdev",setdev);
	UG_INTRO("set_thresh",set_thresh);
	UG_INTRO("lpcin",lpcin);
	p[0]=1; p[1]=10; p[2]=1024; p[3]=1;
	makegen(p,4);  /* store sinewave in array 1 */
	p[0]=2; p[1]=7; p[2]=512; p[3]=0; p[4]=512; p[5]=1; 
	makegen(p,6);
	maxdev=0;
	lowthresh = 0;
	highthresh = 1;
}

double setdev(p,n_args)
float *p;
{
        maxdev = p[0];
}


double
getVoicedAmp(err)
float err;
{
	double sqrt(), sqerr, amp;
	sqerr = sqrt((double) err);
	amp = (sqerr - lowthresh) / (highthresh - lowthresh);
	amp = (amp < 0.0) ? 0.0 : (amp > 1.0) ? 1.0 : amp;
	return amp;
}

double
set_thresh(p, n_args)
float *p;
{
	double log10();
	if(p[1] <= p[0]) {
		fprintf(stderr, "upper thresh must be >= lower!\n");
		closesf();
	}
	lowthresh = p[0];
	highthresh = p[1];
	printf("lower error threshold: %0.6f  upper error threshold: %0.6f\n",
		p[0], p[1]);
}
