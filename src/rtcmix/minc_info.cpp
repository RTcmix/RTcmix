#include <RTcmix.h>
#include "prototypes.h"
#include <ugens.h>
#include <sndlibsupport.h>
#include <sfheader.h>
#include "rtdefs.h"
#include "InputFile.h"
#include "handle.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <PFBusData.h>

#define ALL_CHANS -1

/* These are all for the older disk-based cmix functions. */
extern int	     isopen[NFILES];        /* open status */
extern SFHEADER      sfdesc[NFILES];
extern SFMAXAMP      sfm[NFILES];
extern struct stat   sfst[NFILES];
extern int headersize[NFILES];
extern "C" void sfstats(int fd);       /* defined in sfstats.c */

extern "C" {

double m_sr(double *p, int n_args)
{
  if(!isopen[(int)p[0]]) {
    rtcmix_warn("sr", "You haven't opened file %d yet!", (int)p[0]);
    closesf();
  }
  return(sfsrate(&sfdesc[(int)p[0]]));
}

double m_chans(double *p, int n_args)
{	
  if(!isopen[(int)p[0]]) {
    rtcmix_warn("chans", "You haven't opened file %d yet!", (int)p[0]);
    closesf();
  }
  
  return(sfchans(&sfdesc[(int)p[0]]));
}

double m_class(double *p, int n_args)
{
  if(!isopen[(int)p[0]]) {
    rtcmix_warn("class", "You haven't opened file %d yet!", (int)p[0]);
    closesf();
  }
  return(sfclass(&sfdesc[(int)p[0]]));
}

// Still uses old style soundfile IO arrays, which are now updated with sndlib
// We need to kill that old beast completely!

double m_dur(double *p, int n_args)
{
	int i;
	float dur;
	i = (int) p[0];
	if(!isopen[i]) {
		rtcmix_warn("dur", "You haven't opened file %d yet!", (int)p[0]);
		closesf();
	}
	dur = (float)(sfst[i].st_size - headersize[i])
		 /(float)sfclass(&sfdesc[i])/(float)sfchans(&sfdesc[i])
		 /sfsrate(&sfdesc[i]);
	return(dur);
}

}	// end extern "C"


// Functions for returning info about sound files to the script.  -JGG

extern "C" {
	double filedur(const Arg args[], const int nargs);
	double filechans(const Arg args[], const int nargs);
	double filesr(const Arg args[], const int nargs);
	double filepeak(const Arg args[], const int nargs);
	double filerms(const Arg args[], const int nargs);
	double filedc(const Arg args[], const int nargs);
    Handle filebreakpoints(const Arg args[], const int nargs);
};

double filedur(const Arg args[], const int nargs)
{
    if (nargs != 1) {
		die("filedur", "Usage:  duration = filedur(\"filename\")");
        RTExit(PARAM_ERROR);
   }
	const char *fname = args[0];

	int nchans;
	long nsamps;
	double srate;
	int fd = open_sound_file("filedur", fname, NULL, NULL, NULL,
	                         &srate, &nchans, &nsamps);
	if (fd == -1)
		return -1.0;
   sndlib_close(fd, 0, 0, 0, 0);

   return double(nsamps / nchans) / srate;
}

double filechans(const Arg args[], const int nargs)
{
    if (nargs != 1) {
		die("filechans", "Usage:  chans = filechans(\"filename\")");
        RTExit(PARAM_ERROR);
    }
	const char *fname = args[0];

	int nchans;
	int fd = open_sound_file("filechans", fname, NULL, NULL, NULL,
	                         NULL, &nchans, NULL);
	if (fd == -1)
		return -1.0;
   sndlib_close(fd, 0, 0, 0, 0);

   return double(nchans);
}

double filesr(const Arg args[], const int nargs)
{
    if (nargs != 1) {
		die("filesr", "Usage:  sr = filesr(\"filename\")");
        RTExit(PARAM_ERROR);
    }
	const char *fname = args[0];

	double srate;
	int fd = open_sound_file("filesr", fname, NULL, NULL, NULL,
	                         &srate, NULL, NULL);
	if (fd == -1)
		return -1.0;
   sndlib_close(fd, 0, 0, 0, 0);

   return srate;
}

// helper function for filepeak, filerms, filedc
int findpeakrmsdc(const char *funcname, const char *fname,
	const double starttime, const double endtime, const int chan,
	double *thepeak, double *therms, double *thedc)
{
	int format, dataloc, nchans;
	double srate;
	long nsamps;
	int fd = open_sound_file(funcname, fname, NULL, &format, &dataloc,
	                         &srate, &nchans, &nsamps);
	if (fd == -1)
		return -1;

	long startframe = long((starttime * srate) + 0.5);
	long nframes = (nsamps / nchans) - startframe;

    if (startframe >= nframes) {
        rterror(funcname, "Specified start time is at or past the end of the file.");
        sndlib_close(fd, 0, 0, 0, 0);
        return -1;
    }

    if (endtime != -1.0) {
		long endframe = nframes;
		nframes = long((endtime * srate) + 0.5) + startframe;
		if (nframes > endframe)
			nframes = endframe;
	}

    if (chan != ALL_CHANS && chan >= nchans) {
		die(funcname, "You specified channel %d for a %d-channel file.",
		           chan, nchans);
        sndlib_close(fd, 0, 0, 0, 0);
        RTExit(PARAM_ERROR);
    }
	float peak[nchans];
	long peakloc[nchans];
	double ampavg[nchans];
	double dcavg[nchans];
	double rms[nchans];
	int result = sndlib_findpeak(fd, -1, dataloc, -1, format, nchans,
                    startframe, nframes, peak, peakloc, ampavg, dcavg, rms);
	sndlib_close(fd, 0, 0, 0, 0);
	if (result == -1)
        RTExit(SYSTEM_ERROR);

	if (chan == ALL_CHANS) {
		float maxpeak = 0.0;
		float maxrms = 0.0;
		float maxdc = 0.0;
		for (int i = 0; i < nchans; i++) {
			if (peak[i] > maxpeak)
				maxpeak = peak[i];
			if (rms[i] > maxrms)
				maxrms = rms[i];
			if (dcavg[i] > maxdc)
				maxdc = dcavg[i];
		}
		*thepeak = maxpeak;
		*therms = maxrms;
		*thedc = maxdc;
	}
	else {
		*thepeak = peak[chan];
		*therms = rms[chan];
		*thedc = dcavg[chan];
	}
	return 0;
}

double filepeak(const Arg args[], const int nargs)
{
    if (nargs < 1) {
		die("filepeak", "Usage:  "
		           "peak = filepeak(\"filename\"[, start[, end[, chan]]])");
        RTExit(PARAM_ERROR);
    }
	const char *fname = args[0];
	const double starttime = (nargs > 1) ? args[1] : 0.0;
	const double endtime = (nargs > 2 && double(args[2]) > 0.0) ? args[2] : -1.0;
	const int chan = (nargs > 3) ? int(args[3]) : ALL_CHANS;

	double peak, rms, dc;
	if (findpeakrmsdc("filepeak", (char *) fname, starttime, endtime, chan,
			&peak, &rms, &dc) == -1)
		return -1.0;
	return peak;
}

double filerms(const Arg args[], const int nargs)
{
    if (nargs < 1) {
		die("filerms", "Usage:  "
		           "rms = filerms(\"filename\"[, start[, end[, chan]]])");
        RTExit(PARAM_ERROR);
    }
	const char *fname = args[0];
	const double starttime = (nargs > 1) ? args[1] : 0.0;
	const double endtime = (nargs > 2 && double(args[2]) > 0.0) ? args[2] : -1.0;
	const int chan = (nargs > 3) ? int(args[3]) : ALL_CHANS;

	double peak, rms, dc;
	if (findpeakrmsdc("filerms", (char *) fname, starttime, endtime, chan,
			&peak, &rms, &dc) == -1)
		return -1.0;
	return rms;
}

double filedc(const Arg args[], const int nargs)
{
    if (nargs < 1) {
		die("filedc", "Usage:  "
		           "rms = filedc(\"filename\"[, start[, end[, chan]]])");
        RTExit(PARAM_ERROR);
    }
	const char *fname = args[0];
	const double starttime = (nargs > 1) ? args[1] : 0.0;
	const double endtime = (nargs > 2 && double(args[2]) > 0.0) ? args[2] : -1.0;
	const int chan = (nargs > 3) ? int(args[3]) : ALL_CHANS;

	double peak, rms, dc;
	if (findpeakrmsdc("filedc", (char *) fname, starttime, endtime, chan,
			&peak, &rms, &dc) == -1)
		return -1.0;
	return dc;
}

#define DEFAULT_FRAMESIZE 0.01
#define DEFAULT_THRESHOLD 0.01

Handle filebreakpoints(const Arg args[], const int nargs)
{
    if (nargs < 1) {
        die("filebreakpoints", "Usage:  "
                      "breakpoints_array = filebreakpoints(\"filename\", start, end, framedur, thresh_percent)");
        RTExit(PARAM_ERROR);
    }
    const char *fname = args[0];
    const double starttime = (nargs > 1) ? args[1] : 0.0;
    const double endtime = (nargs > 2 && double(args[2]) > 0.0) ? args[2] : -1.0;
    const double frametime = (nargs > 3) ? double(args[3]) : DEFAULT_FRAMESIZE;     // in fractions of second
    const double thresh = (nargs > 4) ? double(args[4]) : DEFAULT_THRESHOLD;

    int format, dataloc, nchans;
    double srate;
    long nsamps;
    int fd = open_sound_file("filebreakpoints", fname, NULL, &format, &dataloc,
                             &srate, &nchans, &nsamps);
    if (fd == -1)
        return NULL;

    long startframe = long((starttime * srate) + 0.5);
    long nframes = (nsamps / nchans) - startframe;

    if (startframe >= nframes) {
        rterror("filebreakpoints", "Specified start time is at or past the end of the file.");
        sndlib_close(fd, 0, 0, 0, 0);
        return NULL;
    }

    if (endtime != -1.0) {
        nframes = long((endtime * srate) + 0.5) + startframe;
    }

    long windowCount = long((frametime * srate) + 0.5);
    if (windowCount < 2) windowCount = 2;
    long offset = windowCount/2;     // overlap by 50%

    int pointCount = nframes / windowCount;

    // Create Array

    Array *outPoints = (Array *)malloc(sizeof(Array));
    outPoints->len = pointCount;
    outPoints->data = (double *)malloc(pointCount * sizeof(double));
    int pointIndex = 0;

    double runningRMS = 0.0;
    bool wasSilent = true;

    for (long frm = startframe; frm < nframes; frm += offset) {
        float peak[nchans];
        long peakloc[nchans];
        double ampavg[nchans];
        double dcavg[nchans];
        double rms[nchans];
        int result = sndlib_findpeak(fd, -1, dataloc, -1, format, nchans,
                                     frm, windowCount, peak, peakloc, ampavg, dcavg, rms);
        if (result == -1) {
            sndlib_close(fd, 0, 0, 0, 0);
            RTExit(SYSTEM_ERROR);
        }
        double maxpeak = 0.0;
        double maxrms = 0.0;
        for (int i = 0; i < nchans; i++) {
            if (peak[i] > maxpeak)
                maxpeak = peak[i];
            if (rms[i] > maxrms)
                maxrms = rms[i];
        }
        printf("Scanned frames %ld - %ld -- maxpeak %f, maxrms %f\n", frm, frm + windowCount, maxpeak, maxrms);
        double curRMS = maxrms;
        if (curRMS > thresh) {
            if (wasSilent) {
                wasSilent = false;
                printf("Adding up breakpoint %d at time %f\n", pointIndex, frm / srate);
                outPoints->data[pointIndex++] = frm / srate;
            }
        }
        else {
            if (!wasSilent) {
                wasSilent = true;
                printf("Adding down breakpoint %d at time %f\n", pointIndex, frm / srate);
                outPoints->data[pointIndex++] = frm / srate;
            }
        }
    }
    outPoints->len = pointIndex;    // truncate to used portion
    // Wrap Array in Handle, and return.  This will return a 'list' to MinC.
    return createArrayHandle(outPoints);
}

double
RTcmix::input_chans(double *p, int n_args)   /* returns chans for rtinput() files */
{
   int index = get_last_input_index();

   if (index < 0) {
      rtcmix_warn("input_chans", "There are no currently opened input files!");
      return 0.0;
   }
   if (inputFileTable[index].isAudioDevice()) {
      rtcmix_warn("input_chans", "Requesting channels of audio input device, (not sound file)!");
      return 0.0;
   }
   return (inputFileTable[index].channels());
}

double 
RTcmix::input_dur(double *p, int n_args)   /* returns duration for rtinput() files */
{
   int index = get_last_input_index();

   if (index < 0) {
      rtcmix_warn("DUR", "There are no currently opened input files!");
      return 0.0;
   }
   if (inputFileTable[index].isAudioDevice()) {
      rtcmix_warn("input_dur", "Requesting duration of audio input device (not sound file)!");
      return 0.0;
   }
   return (inputFileTable[index].duration());
}

double
RTcmix::input_sr(double *p, int n_args)   /* returns rate for rtinput() files */
{
   int index = get_last_input_index();

   if (index < 0) {
      rtcmix_warn("SR", "There are no currently opened input files!");
      return 0.0;
   }
   return (inputFileTable[index].sampleRate());
}

extern "C" {

/* Note: the old versions of the peak info functions copy peak stats from
   the file header in memory into the sfm[fno] array maintained in sound.c.
   This seems unnecessary, since both are initialized on opening any file
   and updated on endnote. When would they ever be different from the
   perspective of these info functions, which can't be called from Minc
   in the *middle* of an instrument run? Are they also called internally,
   like m_dur?  -JGG
*/ 
double
m_peak(double p[], int n_args)
{
	int      n, fno;
	float    peak, chanpeak;

	fno = (int) p[0];
	if (!isopen[fno]) {
		rtcmix_warn("peak", "You haven't opened file %d yet!", fno);
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
		rtcmix_warn("peak", "File %d has no peak stats!", fno);

	return peak;
}


double
m_left(double p[], int n_args)
{
	int      fno;

	fno = (int) p[0];
	if (!isopen[fno]) {
		rtcmix_warn("left", "You haven't opened file %d yet!", fno);
		closesf();
	}

	if (sfmaxamptime(&sfm[fno]) > 0L)
		return (sfmaxamp(&sfm[fno], 0));    /* for channel 0 */
	else
		rtcmix_warn("left", "File %d has no peak stats!", fno);

	return 0.0;
}


double
m_right(double p[], int n_args)
{
	int      fno;

	fno = (int) p[0];
	if (!isopen[fno]) {
		rtcmix_warn("right", "You haven't opened file %d yet!", fno);
		closesf();
	}

	if (sfmaxamptime(&sfm[fno]) > 0L)
		return (sfmaxamp(&sfm[fno], 1));    /* for channel 1 */
	else
		rtcmix_warn("right", "File %d has no peak stats!", fno);

	return 0.0;
}

} // end of extern "C"


/* Returns peak for <chan> of current RT input file, between <start> and
   <end> times (in seconds). If <chan> is -1, returns the highest peak
   of all the channels. If <end> is 0, sets <end> to duration of file.
*/
float
RTcmix::get_peak(float start, float end, int chan)
{
   int       n, fd, result, nchans, srate, dataloc, format;
   int       index, file_stats_valid=0;
   long      startframe, endframe, nframes, loc;
   long      peakloc[MAXCHANS];
   float     peak[MAXCHANS];
   double    ampavg[MAXCHANS], dcavg[MAXCHANS], rms[MAXCHANS];
   SFComment sfc;

   index = get_last_input_index();

   if (index < 0) {
      rtcmix_warn("get_peak", "There are no currently opened input files!");
      return 0.0;
   }
   if (inputFileTable[index].isAudioDevice()) {
      rtcmix_warn("get_peak", "Requesting peak of audio input device (not sound file)!");
      return 0.0;
   }

   if (end == 0.0)
      end = inputFileTable[index].duration();       /* use end time of file */

   fd = inputFileTable[index].getFD();

// *** If end - start is, say, 60 seconds, less trouble to just analyze file
// than to dig through file header?

   /* Try to use peak stats in file header. */
   if (sndlib_get_header_comment(fd, &sfc) == -1)
      return -0.0;         /* this call prints an err msg */

   if (SFCOMMENT_PEAKSTATS_VALID(&sfc) && sfcomment_peakstats_current(&sfc, fd))
      file_stats_valid = 1;
   else
      file_stats_valid = 0;    /* file written since peak stats were updated */

   format = inputFileTable[index].dataFormat();
   dataloc = inputFileTable[index].dataLocation();
   srate = (int) inputFileTable[index].sampleRate();	// note: casting to int
   nchans = inputFileTable[index].channels();

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
                   startframe, nframes, peak, peakloc, ampavg, dcavg, rms);

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
RTcmix::input_peak(double p[], int n_args)
{
   return get_peak(p[0], p[1], ALL_CHANS);
}


double
RTcmix::left_peak(double p[], int n_args)
{
   return get_peak(p[0], p[1], 0);
}


double
RTcmix::right_peak(double p[], int n_args)
{
   return get_peak(p[0], p[1], 1);
}


extern int sfd[NFILES];

extern "C" {

double
m_info(double *p, int n_args)
{
  sfstats(sfd[(int) p[0]]);
    return 0;
}

}	// end extern "C"


// BGG
extern "C" {
// used to check if a pfbus is connected and to link pfbusses w/ Instruments
	double bus_exists(const Arg args[], const int nargs);
	double bus_link(const Arg args[], const int nargs);
// used to get values from makeconnection("inlet", ...) into an executing score
	double getPFval(const Arg args[], const int nargs);
}

double bus_exists(const Arg args[], const int nargs)
{
    if (nargs != 1) {
		die("bus_exists", "Usage:  val = bus_exists(pfbus #)");
        RTExit(PARAM_ERROR);
    }
	int connection = (int)args[0];
	int is_connected = PFBusData::pfbus_is_connected[connection];

	if (is_connected == 1) return 1.0;
	else return 0.0;
}


double bus_link(const Arg args[], const int nargs)
{
    if (nargs != 1) {
		die("bus_link", "Usage:  val = bus_link(pfbus #)");
        RTExit(PARAM_ERROR);
    }

	PFBusData::connect_val = (int)args[0];
	return 1.0;
}

double getPFval(const Arg args[], const int nargs)
{
	PField *PF = args[0];
	if (PF == NULL) {
		rtcmix_warn("getPFVal", "PField is not initialized");
		return(0.0);
	}
	return (PF->doubleValue());
}
