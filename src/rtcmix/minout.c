#include <ugens.h>
#include <sfheader.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

extern float SR;

/* these defined in sound.c */
extern int  sfd[NFILES];         /* soundfile descriptors */
extern int  pointer[NFILES];     /* to be used as pointer within sound buffer */
extern int  bufsize[NFILES];     /* word length of buffer */
extern int  status[NFILES];
extern char peakoff[NFILES];
extern char *sndbuf[NFILES];     /* address of buffer */
extern char wipe_is_off[NFILES]; /* this is for wipeout */
extern long filepointer[NFILES]; /* to save current pointer in file */

extern SFHEADER      sfdesc[NFILES];


int
inrepos(int samps, int fno)
{
        int seeking,amt;

        amt = samps * sfchans(&sfdesc[fno]) + pointer[fno];

        if ( (amt >= 0) && (amt < bufsize[fno]) ) {
                pointer[fno] = amt;     /* reposition inside buffer */
                return(pointer[fno]);
                }
        
        seeking = (amt - bufsize[fno]) * sfclass(&sfdesc[fno]);

        if ( (filepointer[fno] = lseek(sfd[fno],seeking,1)) == -1 ) {
                fprintf(stderr,"CMIX: Bad inrepos lseek, file: %d\n",fno);
                closesf();
                }

        _readit(fno);
        pointer[fno] = 0;
        return(filepointer[fno]);
}

int
outrepos(int samps, int fno)
{
        int seeking,amt;

        if(!status[fno]) {
                fprintf(stderr,"file %d is write protected!\n",fno);
                perror("write");
                closesf();
                }

        amt = samps * sfchans(&sfdesc[fno]) + pointer[fno];

        if ( (amt >= 0) && (amt < bufsize[fno]) ) {
                pointer[fno] = amt;     /* inside current buffer        */
                return(pointer[fno]);
                }
        
        if(wipe_is_off) _backup(fno);
        if(!peakoff[fno]) _chkpeak(fno);
        _writeit(fno);          /* write out current buffer */
        seeking = (amt - bufsize[fno]) * sfclass(&sfdesc[fno]);

        if ( (filepointer[fno] = lseek(sfd[fno],seeking,1)) == -1 ) {
                fprintf(stderr,"CMIX: bad outrepos lseek, file: %d\n",fno);
                closesf();
                }
        
        if(wipe_is_off) _readit(fno);
        pointer[fno] = 0;
        return(filepointer[fno]);
}

