#include <globals.h>
#include <prototypes.h>
#include <ugens.h>
#include <sndlibsupport.h>
#include <sfheader.h>
#include "rtdefs.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>


/* These are all for the older disk-based cmix functions. */
extern int	     isopen[NFILES];        /* open status */
extern SFHEADER      sfdesc[NFILES];
extern SFMAXAMP      sfm[NFILES];
extern struct stat   sfst[NFILES];
extern int headersize[NFILES];
extern void sfstats(int fd);       /* defined in sfstats.c */


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

double m_CHANS(float *p, int n_args)   /* returns chans for rtinput() files */
{
   int index = get_last_input_index();

   if (index < 0) {
      fprintf(stderr, "There are no currently opened input files!\n");
      return 0.0;
   }
   if (inputFileTable[index].is_audio_dev) {
      fprintf(stderr, "WARNING: Requesting channels of audio input device "
                      "(not sound file)!\n");
      return 0.0;
   }
   return (inputFileTable[index].chans);
}

double m_DUR(float *p, int n_args)   /* returns duration for rtinput() files */
{
   int index = get_last_input_index();

   if (index < 0) {
      fprintf(stderr, "There are no currently opened input files!\n");
      return 0.0;
   }
   if (inputFileTable[index].is_audio_dev) {
      fprintf(stderr, "WARNING: Requesting duration of audio input device "
                      "(not sound file)!\n");
      return 0.0;
   }
   return (inputFileTable[index].dur);
}

double m_SR(float *p, int n_args)   /* returns rate for rtinput() files */
{
   int index = get_last_input_index();

   if (index < 0) {
      fprintf(stderr, "There are no currently opened input files!\n");
      return 0.0;
   }
//   if (inputFileTable[index].is_audio_dev) {
//     fprintf(stderr, "WARNING: Requesting duration of audio input device "
//                    "(not sound file)!\n");
//   return 0.0;
//   }
   return (inputFileTable[index].srate);
}

/* Note: the old versions of the peak info functions copy peak stats from
   the file header in memory into the sfm[fno] array maintained in sound.c.
   This seems unnecessary, since both are initialized on opening any file
   and updated on endnote. When would they ever be different from the
   perspective of these info functions, which can't be called from Minc
   in the *middle* of an instrument run? Are they also called internally,
   like m_dur?  -JGG
*/ 
double
m_peak(float p[], int n_args)
{
	int      n, fno;
	float    peak, chanpeak;

	fno = p[0];
	if (!isopen[fno]) {
		fprintf(stderr, "You haven't opened file %d yet!\n", fno);
		closesf();
	}

	peak = 0.0;

	if (sfmaxamptime(&sfm[fno]) > 0L) {
		for (n = 0; n < sfchans(&sfdesc[fno]); n++) {
			chanpeak = sfmaxamp(&sfm[fno], n);
			if (chanpeak > peak)
				peak = chanpeak;
		}
	}
	else
		fprintf(stderr, "File %d has no peak stats!\n", fno);

	return peak;
}


double
m_left(float p[], int n_args)
{
	int      fno;

	fno = p[0];
	if (!isopen[fno]) {
		fprintf(stderr, "You haven't opened file %d yet!\n", fno);
		closesf();
	}

	if (sfmaxamptime(&sfm[fno]) > 0L)
		return (sfmaxamp(&sfm[fno], 0));    /* for channel 0 */
	else
		fprintf(stderr, "File %d has no peak stats!\n", fno);

	return 0.0;
}


double
m_right(float p[], int n_args)
{
	int      fno;

	fno = p[0];
	if (!isopen[fno]) {
		fprintf(stderr, "You haven't opened file %d yet!\n", fno);
		closesf();
	}

	if (sfmaxamptime(&sfm[fno]) > 0L)
		return (sfmaxamp(&sfm[fno], 1));    /* for channel 1 */
	else
		fprintf(stderr, "File %d has no peak stats!\n", fno);

	return 0.0;
}



#define ALL_CHANS -1

/* Returns peak for <chan> of current RT input file, between <start> and
   <end> times (in seconds). If <chan> is -1, returns the highest peak
   of all the channels. If <end> is 0, sets <end> to duration of file.
*/
static float
get_peak(float start, float end, int chan)
{
   int       n, fd, result, nchans, srate, dataloc, format;
   int       index, file_stats_valid=0;
   long      startframe, endframe, nframes, loc;
   long      peakloc[MAXCHANS];
   float     peak[MAXCHANS];
   SFComment sfc;

   index = get_last_input_index();

   if (index < 0) {
      fprintf(stderr, "There are no currently opened input files!\n");
      return 0.0;
   }
   if (inputFileTable[index].is_audio_dev) {
      fprintf(stderr, "WARNING: Requesting peak of audio input device "
                      "(not sound file)!\n");
      return 0.0;
   }

   if (end == 0.0)
      end = inputFileTable[index].dur;       /* use end time of file */

   fd = inputFileTable[index].fd;

// *** If end - start is, say, 60 seconds, less trouble to just analyze file
// than to dig through file header?

   /* Try to use peak stats in file header. */
   if (sndlib_get_header_comment(fd, &sfc) == -1)
      return -0.0;         /* this call prints an err msg */

   if (SFCOMMENT_PEAKSTATS_VALID(&sfc) && sfcomment_peakstats_current(&sfc, fd))
      file_stats_valid = 1;
   else
      file_stats_valid = 0;    /* file written since peak stats were updated */

   format = inputFileTable[index].data_format;
   dataloc = inputFileTable[index].data_location;
   srate = inputFileTable[index].srate;
   nchans = inputFileTable[index].chans;

   startframe = (long)(start * srate + 0.5);
   endframe = (long)(end * srate + 0.5);
   nframes = (endframe - startframe) + 1;

   if (file_stats_valid) {
      int c = 0;
      if (chan == ALL_CHANS) {
         float max = 0.0;
         for (n = 0; n < nchans; n++) {
            if (sfc.peak[n] > max) {
               max = sfc.peak[n];
               c = n;
            }
         }
         loc = sfc.peakloc[c];
      }
      else {
         loc = sfc.peakloc[chan];
         c = chan;
      }
      if (loc >= startframe && loc <= endframe)
         return sfc.peak[c];
   }

   /* NOTE: Might get here even if file_stats_valid. */
   result = sndlib_findpeak(fd, -1, dataloc, -1, format, nchans,
                            startframe, nframes, peak, peakloc);

   if (chan == ALL_CHANS) {
      float max = 0.0;
      for (n = 0; n < nchans; n++) {
         if (peak[n] > max)
            max = peak[n];
      }
      return max;
   }
   else
      return peak[chan];
}


double
m_PEAK(float p[], int n_args)
{
   return get_peak(p[0], p[1], ALL_CHANS);
}


double
m_LEFT_PEAK(float p[], int n_args)
{
   return get_peak(p[0], p[1], 0);
}


double
m_RIGHT_PEAK(float p[], int n_args)
{
   return get_peak(p[0], p[1], 1);
}


extern int sfd[NFILES];

double
m_info(p,n_args)
float *p;
{
  sfstats(sfd[(int) p[0]]);
    return 0;
}
