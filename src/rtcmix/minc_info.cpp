#include "../H/ugens.h"
#include "../H/sfheader.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../rtstuff/rtdefs.h"


extern int	     isopen[NFILES];        /* open status */
extern SFHEADER      sfdesc[NFILES];
extern SFMAXAMP      sfm[NFILES];
extern struct stat   sfst[NFILES];
extern int headersize[NFILES];

extern int rtInputIndex;
extern InputDesc inputFileTable[];


double m_sr(p,n_args)
float *p;
{
  if(!isopen[(int)p[0]]) {
    fprintf(stderr, "You haven't opened file %d yet!\n", (int)p[0]);
    closesf();
  }
  return(sfsrate(&sfdesc[(int)p[0]]));
}

double m_chans(p,n_args)
float *p;
{	
  if(!isopen[(int)p[0]]) {
    fprintf(stderr, "You haven't opened file %d yet!\n", (int)p[0]);
    closesf();
  }
  
  return(sfchans(&sfdesc[(int)p[0]]));
}

double m_class(p,n_args)
float *p;
{
  if(!isopen[(int)p[0]]) {
    fprintf(stderr, "You haven't opened file %d yet!\n", (int)p[0]);
    closesf();
  }
  return(sfclass(&sfdesc[(int)p[0]]));
}

// Still uses old style soundfile IO arrays, which are now updated with sndlib
// We need to kill that old beast completely!

double m_dur(p,n_args)
float *p;
{
	int i;
	float dur;
	i = p[0];
	if(!isopen[i]) {
		fprintf(stderr, "You haven't opened file %d yet!\n", i);
		closesf();
	}
	dur = (float)(sfst[i].st_size - headersize[i])
		 /(float)sfclass(&sfdesc[i])/(float)sfchans(&sfdesc[i])
		 /sfsrate(&sfdesc[i]);
	return(dur);
}

double m_DUR(float *p, int n_args)   /* returns duration for rtinput() files */
{
  if (rtInputIndex < 0) {
    fprintf(stderr, "There are no currently opened input files!\n");
    return 0.0;
  }
  return(inputFileTable[rtInputIndex].dur);
}

double m_peak(p,n_args)
float *p;
{
	char *cp,*getsfcode();
	int i,j;
	float peak;
	j = p[0];
	if(!isopen[j]) {
		fprintf(stderr, "You haven't opened file %d yet!\n", j);
		closesf();
	}
	cp = getsfcode(&sfdesc[j],SF_MAXAMP);
	bcopy(cp + sizeof(SFCODE), (char *) &sfm[j], sizeof(SFMAXAMP));
	if(cp != NULL) {
		for(i=0,peak=0; i<sfchans(&sfdesc[j]); i++)
			if(sfmaxamp(&sfm[j],i) > peak) peak=sfmaxamp(&sfm[j],i);
		return(peak);
		}
	return(0.);
}

double m_left(p,n_args)
float *p;
{
	char *cp,*getsfcode();
	int i,j;
	j = p[0];
	if(!isopen[j]) {
		fprintf(stderr, "You haven't opened file %d yet!\n", j);
		closesf();
	}
	cp = getsfcode(&sfdesc[j],SF_MAXAMP);
	bcopy(cp + sizeof(SFCODE), (char *) &sfm[j], sizeof(SFMAXAMP));
	if(cp != NULL) 
		return(sfmaxamp(&sfm[j],0));
	return(0.);
}

double m_right(p,n_args)
float *p;
{
	char *cp,*getsfcode();
	int i,j;
	j = p[0];
	if(!isopen[j]) {
		fprintf(stderr, "You haven't opened file %d yet!\n", j);
		closesf();
	}
	if(sfchans(&sfdesc[j]) == 1) return(0);
	cp = getsfcode(&sfdesc[j],SF_MAXAMP);
	bcopy(cp + sizeof(SFCODE), (char *) &sfm[j], sizeof(SFMAXAMP));
	if(cp != NULL) 
		return(sfmaxamp(&sfm[j],1));
	return(0.);
}

extern int sfd[NFILES];

double
m_info(p,n_args)
float *p;
{
  sfstats(sfd[(int) p[0]]);
    return 0;
}
