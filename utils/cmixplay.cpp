/* RTcmix  - Copyright (C) 2001  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/* revision of Dave Topper's play program, by John Gibson, 6/99.
   rev'd again, for v2.3  -JGG, 2/26/00.
   rev'd again, for OSX support  -JGG, 12/01.
   rev'd again, to work with Doug's AudioDevice class  -JGG, 6/04.
*/
#if defined(LINUX) || defined(MACOSX)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <errno.h>
#include <assert.h>
#include <byte_routines.h>
#include <sndlibsupport.h>
#include <AudioDevice.h>
#include <audio_devices.h>


#define PROGNAME  "cmixplay"

/* Tradeoff: larger BUF_FRAMES makes playback more robust on a loaded machine
   or over a network, at the cost of elapsed seconds display running ahead of
   playback.  If user runs with robust flag (-r), code below increases buffer
   params by ROBUST_FACTOR.
*/

#ifdef LINUX
	#include <values.h>
   #define DEFAULT_DEVICE_NAME "plughw"
   #define BUF_FRAMES          1024
   #define ROBUST_FACTOR       4
#endif

#ifdef MACOSX
	#include <limits.h>
   #define DEFAULT_DEVICE_NAME NULL
   #define BUF_FRAMES          1024
   // 4 glitches ... why?   XXX: still true?
   #define ROBUST_FACTOR       2
#endif

#define SKIP_SECONDS        4.0f    // for fast-forward and rewind
#define MARK_PRECISION      2       // digits after decimal point to print
                                    // for mark
#define MARK_DELAY          0.05f   // report mark time earlier by this much,
                                    // to make up for delay when typing key
#define FACTOR_INCREMENT    0.1f    // for hotkey volume changes

#define ALL_CHANS           -1

//#define DEBUG

#ifdef DEBUG
   #define DPRINT(msg)                    printf((msg))
   #define DPRINT1(msg, arg)              printf((msg), (arg))
   #define DPRINT2(msg, arg1, arg2)       printf((msg), (arg1), (arg2))
   #define DPRINT3(msg, arg1, arg2, arg3) printf((msg), (arg1), (arg2), (arg3))
#else
   #define DPRINT(msg)
   #define DPRINT1(msg, arg)
   #define DPRINT2(msg, arg1, arg2)
   #define DPRINT3(msg, arg1, arg2, arg3)
#endif


#define UNSUPPORTED_DATA_FORMAT_MSG "\
%s: samples in an unsupported data format. \n\
(%s will play 16-bit linear, 24-bit linear or 32-bit floating point samples, \n\
in either byte order.) \n"

#define NEED_FACTOR_MSG "\
This floating-point file doesn't have up-to-date peak stats in its header. \n\
Either run sndpeak on the file or supply a rescale factor when you play.   \n\
(And watch out for your ears!)\n"

#define CLIPPING_FORCE_FACTOR_MSG "\
Your rescale factor (%g) would cause clipping.  If you really want to  \n\
play the file with this factor anyway, use the \"--force\" flag on the \n\
command line.  But be careful with your ears!\n"

#define NO_STATS_FORCE_FACTOR_MSG "\
Since this file header has no up-to-date peak stats, there's no way to  \n\
guarantee that your rescale factor won't cause painfully loud playback. \n\
If you really want to play the file with this factor anyway, use the    \n\
\"--force\" flag on the command line.  But be careful with your ears!\n"


typedef enum {
   StatusError = -1,
   StatusGood,
   StatusAbort
} Status;

typedef enum {
   StateStopped,
   StatePlaying,
   StatePaused
} State;

typedef enum {
   TimeFormatSeconds,
   TimeFormatMinutesSeconds
} TimeFormat;

static bool audioDone = false;

static const char *make_time_string(const float seconds, const int precision);
static const float get_seconds(const char timestr[], TimeFormat *format);
static Status set_input_mode();
static void reset_input_mode();

class Player {

public:
   Player(const char       *fileName,
          const char       *deviceName,
          const float      startTime,
          const float      endTime,
          const int        playChan,
          const int        requestedBufFrames,   
          const float      rescaleFactor,
          const bool       forcePeak,
          const bool       useHotKeys,
          const float      hotKeySkipTime,
          const bool       quiet,
          const TimeFormat timeFormat
         );
   ~Player();

   Status      configure();
   Status      play();
   State       getState() const { return _state; }
   
   static bool playCallback(AudioDevice *device, void *arg);
   static bool stopPlayCallback(AudioDevice *device, void *arg);

protected:
   int         readBuffer();
   void        closeFiles();
   Status      doHotKeys(int framesRead);
   void        printTime();
   float       *getOutBuf() const { return _outBuf; }

private:
   const char  *_fileName;
   const char  *_deviceName;
   float       _startTime;
   float       _endTime;
   int         _playChan;
   int         _deviceFrames;
   float       _factor;
   bool        _force;
   bool        _useHotKeys;
   float       _hotKeySkipTime;
   bool        _quiet;
   TimeFormat  _timeFormat;

   int         _fd;
   int         _dataFormat;
   int         _sRate;
   int         _fileChans;
   int         _datumSize;
   int         _fileSamps;    // number of samples, not frames, in file
   float       _fileDur;
   int         _dataLocation;
   bool        _isFloatFile;
   bool        _is24bitFile;
   bool		   _is32bitFile;
   bool        _swap;
   int         _framesRead;
   int         _bufSamps;
   int         _endFrame;
   float       _bufStartTime;
   int         _bufStartFrame;
   long        _skipBytes;
   int         _curSecond;
   void        *_inBuf;       // filled from file read
   float       *_outBuf;      // passed to audio device
   State       _state;
   AudioDevice *_device;

   Status      openInputFile();
   Status      computeRescaleFactor();
   Status      openAudioDevice();
   void        printStats();
};


// ------------------------------------------------------------------ Player ---
Player::Player(
   const char        *fileName,
   const char        *deviceName,
   const float       startTime,
   const float       endTime,
   const int         playChan,
   const int         requestedBufFrames,   
   const float       rescaleFactor,
   const bool        forcePeak,
   const bool        useHotKeys,
   const float       hotKeySkipTime,
   const bool        quiet,
   const TimeFormat  timeFormat)
   : _fileName(fileName), _deviceName(deviceName), _startTime(startTime),
     _endTime(endTime), _playChan(playChan), _deviceFrames(requestedBufFrames),
     _factor(rescaleFactor), _force(forcePeak), _useHotKeys(useHotKeys),
     _hotKeySkipTime(hotKeySkipTime), _quiet(quiet), _timeFormat(timeFormat)
{
   _inBuf = NULL;
   _outBuf = NULL;
   _device = NULL;
   _state = StateStopped;
}


// ----------------------------------------------------------------- ~Player ---
Player::~Player()
{
   delete [] (float *) _inBuf;
   delete [] _outBuf;
}


// --------------------------------------------------------------- configure ---
Status
Player::configure()
{
   Status status = openInputFile();
   if (status != StatusGood)
      return status;

   // more input validation

   if (_startTime >= _fileDur) {
      fprintf(stderr, "Start time must be less than duration of file.\n");
      return StatusError;
   }
   if (_endTime > 0.0 && _endTime > _fileDur) {
      if (!_quiet)
         printf("Note: Your end time was later than the end of file.\n");
      // but continue anyway
      _endTime = 0.0;
   }
   if (_playChan >= _fileChans) {
      fprintf(stderr, "You asked to play channel %d of a %d-channel file.\n",
              _playChan, _fileChans);
      return StatusError;
   }

   status = computeRescaleFactor();
   if (status != StatusGood)
      return status;

   status = openAudioDevice();
   if (status != StatusGood)
      return status;

   _bufSamps = _deviceFrames * _fileChans;

   // allocate input and output buffers
   _inBuf = (void *) new char [_bufSamps * _datumSize];
   _outBuf = new float [_bufSamps];

   printStats();

   return StatusGood;
}


// ----------------------------------------------------------- openInputFile ---
Status
Player::openInputFile()
{
   struct stat statbuf;

   // see if file exists and we can read it
   _fd = open(_fileName, O_RDONLY);
   if (_fd == -1) {
      fprintf(stderr, "%s: %s\n", _fileName, strerror(errno));
      return StatusError;
   }

   // make sure it's a regular file or symbolic link
   if (fstat(_fd, &statbuf) == -1) {
      fprintf(stderr, "%s: %s\n", _fileName, strerror(errno));
      return StatusError;
   }
   if (!S_ISREG(statbuf.st_mode) && !S_ISLNK(statbuf.st_mode)) {
      fprintf(stderr, "\"%s\" is not a regular file or a link.\n", _fileName);
      return StatusError;
   }

   // read header and gather info
   if (sndlib_read_header(_fd) == -1) {
      fprintf(stderr, "Can't read \"%s\"!\n", _fileName);
      return StatusError;
   }
   int headerType = mus_header_type();
   if (NOT_A_SOUND_FILE(headerType)) {
      fprintf(stderr, "\"%s\" is probably not a sound file\n", _fileName);
      return StatusError;
   }
   _dataFormat = mus_header_format();
   if (!SUPPORTED_DATA_FORMAT(_dataFormat)) {
      fprintf(stderr, UNSUPPORTED_DATA_FORMAT_MSG, _fileName, PROGNAME);
      return StatusError;
   }
   _isFloatFile = IS_FLOAT_FORMAT(_dataFormat);
   _is24bitFile = IS_24BIT_FORMAT(_dataFormat);
   _is32bitFile = IS_32BIT_FORMAT(_dataFormat);
#if MUS_LITTLE_ENDIAN
   _swap = IS_BIG_ENDIAN_FORMAT(_dataFormat);
#else
   _swap = IS_LITTLE_ENDIAN_FORMAT(_dataFormat);
#endif
   _dataLocation = mus_header_data_location();
   _sRate = mus_header_srate();
   _fileChans = mus_header_chans();
   _fileSamps = mus_header_samples();
   _fileDur = (float) (_fileSamps / _fileChans) / (float) _sRate;
   _datumSize = mus_header_data_format_to_bytes_per_sample();

   return StatusGood;
}


// ---------------------------------------------------- computeRescaleFactor ---
Status
Player::computeRescaleFactor()
{
   SFComment sfc;

   if (sndlib_get_current_header_comment(_fd, &sfc) == -1) {
      fprintf(stderr, "Can't read header comment!\n");
      return StatusError;
   }
   int validStats = (SFCOMMENT_PEAKSTATS_VALID(&sfc)
                  && sfcomment_peakstats_current(&sfc, _fd));

   if (validStats) {
      float peak = 0.0;
      for (int n = 0; n < _fileChans; n++)
         if (sfc.peak[n] > peak)
            peak = sfc.peak[n];
      if (peak > 0.0) {
         float tmpFactor = 32767.0 / peak;
         if (_factor) {
            if (_factor > tmpFactor && !_force) {
               fprintf(stderr, CLIPPING_FORCE_FACTOR_MSG, _factor);
               return StatusError;
            }
         }
         else
            _factor = tmpFactor > 1.0 ? 1.0 : tmpFactor;
      }
      else
         validStats = 0;            // better not to believe this peak
   }

   if (!validStats) {               // NOTE: validStats can change in prev block
      if (_factor == 0.0) {
         if (_isFloatFile) {
            fprintf(stderr, NEED_FACTOR_MSG);
            return StatusError;
         }
         else
            _factor = 1.0;
      }
      else if (!_force) {
         if (_isFloatFile || _factor > 1.0) {
            fprintf(stderr, NO_STATS_FORCE_FACTOR_MSG);
            return StatusError;
         }
      }
   }

   return StatusGood;
}


// --------------------------------------------------------- openAudioDevice ---
static const int numBuffers = 2;    // number of audio buffers to queue up

Status
Player::openAudioDevice()
{
   _device = createAudioDevice(NULL, _deviceName, false, true);
   if (_device == NULL) {
      fprintf(stderr, "Failed to create audio device \"%s\".\n", _deviceName);
      return StatusError;
   }

   // We hand the device an interleaved floating point buffer.
   int audioFormat = NATIVE_FLOAT_FMT | MUS_INTERLEAVED;
   int openMode = AudioDevice::Playback;

   _device->setFrameFormat(audioFormat, _fileChans);

   if (_device->open(openMode, audioFormat, _fileChans, _sRate) < 0) {
      fprintf(stderr, "%s\n", _device->getLastError());
      return StatusError;
   }

   int reqsize = _deviceFrames;
   int reqcount = numBuffers;
   if (_device->setQueueSize(&reqsize, &reqcount) < 0) {
      fprintf(stderr, "%s\n", _device->getLastError());
      return StatusError;
   }
   if (reqsize != _deviceFrames) {
      if (!_quiet)
         printf("Note: buffer size reset by audio device from %d to %d.",
                                                      _deviceFrames, reqsize);
      _deviceFrames = reqsize;
   }

   return StatusGood;
}


// -------------------------------------------------------------- printStats ---
void
Player::printStats()
{
   if (!_quiet) {
      printf("File: %s\n", _fileName);
      printf("Rate: %d Hz   Chans: %d\n", _sRate, _fileChans);
      printf("Duration: %.*f seconds [%s]\n", MARK_PRECISION, _fileDur,
                                 make_time_string(_fileDur, MARK_PRECISION));
      if (_startTime > 0.0)
         printf("Skipping %g seconds.\n", _startTime);
      if (_endTime > 0.0)
         printf("Ending at %g seconds.\n", _endTime);
      printf("Rescale factor: %g\n", _factor);
      printf("Time: ");
      fflush(stdout);
   }
}


// --------------------------------------------------------------- doHotKeys ---
Status
Player::doHotKeys(int framesRead)
{
   if (_useHotKeys) {
      int skipBytes = 0;
      char c = 0;
      long bytesRead = read(STDIN_FILENO, &c, 1);
      if (bytesRead > 0) {
         int skip = 0;

         if (c == '\004')        // control-D
            return StatusAbort;
         else if (c == 'f') {    // fast-forward
            skip = 1;
            int skipFrames = (int) (_hotKeySkipTime * _sRate);
            _skipBytes = (long) (skipFrames * _fileChans * _datumSize);
            _bufStartFrame += skipFrames;
            if (!_quiet) {
               _bufStartTime += _hotKeySkipTime;
               _curSecond += (int) _hotKeySkipTime;
               printf("\n");
            }
         }
         else if (c == 'r') {    // rewind
            skip = 1;
            int skipFrames = (int) (_hotKeySkipTime * _sRate);
            _skipBytes = -(long) (skipFrames * _fileChans * _datumSize);
            _bufStartFrame -= skipFrames;
            if (!_quiet) {
               _bufStartTime -= _hotKeySkipTime;
               _curSecond -= (int) _hotKeySkipTime;
               printf("\n");
            }
         }
         else if (c == 'm') {    // mark
            float markTime = _bufStartTime - MARK_DELAY;
            printf("\nMARK: %.*f [%s]\n", MARK_PRECISION, markTime,
                              make_time_string(markTime, MARK_PRECISION));
            fflush(stdout);
         }
         else if (c == 't') {    // toggle time display format
            if (_timeFormat == TimeFormatSeconds)
               _timeFormat = TimeFormatMinutesSeconds;
            else
               _timeFormat = TimeFormatSeconds;
         }
         else if (c == 'p') {    // toggle pause
            if (_state == StatePlaying) {
               _state = StatePaused;
               if (!_quiet)
                  printf("\nPaused...[type 'p' to resume]\n");
            }
            else
               _state = StatePlaying;
         }
         else if (c == '-') {    // decrease volume (factor)
            _factor -= FACTOR_INCREMENT;
            if (_factor < 0.0)
               _factor = 0.0;
            printf("\nRescale factor: %.2f\n", _factor);
         }
         else if (c == '+' || c == '=') { // increase volume
            _factor += FACTOR_INCREMENT;
            printf("\nRescale factor: %.2f\n", _factor);
         }

         if (skip) {
            off_t curloc = lseek(_fd, 0, SEEK_CUR);
            if (curloc + _skipBytes <= _dataLocation) {
               if (!_quiet) {
                  _bufStartTime = 0.0;
                  _curSecond = 0;
               }
               if (lseek(_fd, _dataLocation, SEEK_SET) == -1) {
                  perror("lseek");
                  return StatusError;
               } 
               _bufStartFrame = -framesRead;
            }
            else if (lseek(_fd, _skipBytes, SEEK_CUR) == -1) {
               perror("lseek");
               return StatusError;
            }
            if (_state == StatePaused) {
               DPRINT("calling printTime while paused\n");
               printTime();
            }
         }
      }
   }

   return StatusGood;
}


// --------------------------------------------------------------- printTime ---
void
Player::printTime()
{
   if (_quiet)
      return;

   if (_bufStartTime >= _curSecond || _state == StatePaused) {
      if (_timeFormat == TimeFormatMinutesSeconds)
         printf("%s ", make_time_string((float) _curSecond, 0));
      else
         printf("%d ", _curSecond);
      fflush(stdout);
      if (_state == StatePlaying)
         _curSecond++;
   }
}


// -------------------------------------------------------------- readBuffer ---
int
Player::readBuffer()
{
   long bytesRead;

   DPRINT3("bufStartFrame=%d, deviceFrames=%d, endFrame=%d\n",
                                 _bufStartFrame, _deviceFrames, _endFrame);

   if (_bufStartFrame + _deviceFrames > _endFrame) {      // last buffer
      int samps = (_endFrame - _bufStartFrame) * _fileChans;
      if (samps <= 0)
         return 0;               // _endFrame reached
      DPRINT1("last buffer: requesting %d frames\n", samps / _fileChans);
      bytesRead = read(_fd, _inBuf, samps * _datumSize);
   }
   else
      bytesRead = read(_fd, _inBuf, _bufSamps * _datumSize);
   if (bytesRead == -1) {
      perror("read");
      return -1;
   }
   if (bytesRead == 0)          // EOF
      return 0;

   int sampsRead = bytesRead / _datumSize;
   int framesRead = sampsRead / _fileChans;

   if (_isFloatFile) {
      float *bufp = (float *) _inBuf;
      if (_swap) {
         for (int i = 0; i < sampsRead; i++) {
            byte_reverse4(&bufp[i]);
            _outBuf[i] = bufp[i] * _factor;
         }
      }
      else {
         for (int i = 0; i < sampsRead; i++)
            _outBuf[i] = bufp[i] * _factor;
      }
   }
   else if (_is32bitFile) {	// 32bit int
  	 static const float intScale = 32768.0f/INT_MAX;
      int *bufp = (int *) _inBuf;
      if (_swap) {
         for (int i = 0; i < sampsRead; i++) {
            bufp[i] = reverse_int4(&bufp[i]);
            _outBuf[i] = (float) (intScale * _factor * (float) bufp[i]);
         }
      }
      else {
         for (int i = 0; i < sampsRead; i++)
            _outBuf[i] = (float) (intScale * _factor * (float) bufp[i]);
      }
   }
   else if (_is24bitFile) {
      unsigned char *bufp = (unsigned char *) _inBuf;
      int j;
      if (_dataFormat == MUS_L24INT) {
         for (int i = j = 0; i < sampsRead; i++, j += _datumSize) {
            float samp = (float) (((bufp[j + 2] << 24)
                                 + (bufp[j + 1] << 16)
                                 + (bufp[j] << 8)) >> 8);
            samp *= _factor;
            _outBuf[i] = samp / (float) (1 << 8);
         }
      }
      else {   // _dataFormat == MUS_B24INT
         for (int i = j = 0; i < sampsRead; i++, j += _datumSize) {
            float samp = (float) (((bufp[j] << 24)
                                 + (bufp[j + 1] << 16)
                                 + (bufp[j + 2] << 8)) >> 8);
            samp *= _factor;
            _outBuf[i] = samp / (float) (1 << 8);
         }
      }
   }
   else {      // 16bit int
      short *bufp = (short *) _inBuf;
      if (_swap) {
         for (int i = 0; i < sampsRead; i++) {
            bufp[i] = reverse_int2(&bufp[i]);
            _outBuf[i] = (float) (_factor * (float) bufp[i]);
         }
      }
      else {
         for (int i = 0; i < sampsRead; i++)
            _outBuf[i] = (float) (_factor * (float) bufp[i]);
      }
   }

   for (int i = sampsRead; i < _bufSamps; i++)
      _outBuf[i] = 0.0;

   // Not efficient, but easy way to play just 1 of several chans.
   if (_playChan > ALL_CHANS) {
      for (int i = 0; i < sampsRead; i += _fileChans) {
         float samp = _outBuf[i + _playChan];
         for (int n = 0; n < _fileChans; n++)
            _outBuf[i + n] = samp;
      }
   }

   _bufStartFrame += framesRead;
   _bufStartTime += (float) framesRead / (float) _sRate;

   return framesRead;
}


// ------------------------------------------------------------ playCallback ---
bool
Player::playCallback(AudioDevice *device, void *arg)
{
   int framesRead = 0;
   Player *player = (Player *) arg;

   if (player->getState() == StatePlaying) {
      framesRead = player->readBuffer();
      if (framesRead <= 0)
         return false;

      int framesWritten = device->sendFrames(player->getOutBuf(), framesRead);
      if (framesWritten != framesRead)
         fprintf(stderr, "Error: %s\n", device->getLastError());

      player->printTime();
   }
   Status status = player->doHotKeys(framesRead);
   if (status != StatusGood)
      return false;

   return true;
}


// -------------------------------------------------------------- closeFiles ---
void
Player::closeFiles()
{
   close(_fd);
}


// -------------------------------------------------------- stopPlayCallback ---
bool
Player::stopPlayCallback(AudioDevice *device, void *arg)
{
   Player *player = (Player *) arg;

   device->close();
   player->closeFiles();
   audioDone = true;

   return true;
}


// -------------------------------------------------------------------- play ---
Status
Player::play()
{
   int startFileFrame = 0;

   // init variables used in playback loop

   if (_startTime > 0.0) {
      startFileFrame = (int) (_startTime * _sRate);
      _skipBytes = (long) (startFileFrame * _fileChans * _datumSize);
   }
   else
      _skipBytes = 0;

   if (_endTime > 0.0)
      _endFrame = (int) (_endTime * _sRate);
   else
      _endFrame = _fileSamps / _fileChans;

   _bufStartFrame = startFileFrame;
   _bufStartTime = _startTime;
   _curSecond = (int) _bufStartTime;

   // seek to start time on input file
   if (lseek(_fd, _dataLocation + _skipBytes, SEEK_SET) == -1) {
      perror("lseek");
      return StatusError;
   }

   _state = StatePlaying;

   _device->setStopCallback(Player::stopPlayCallback, (void *) this);

   if (_device->start(Player::playCallback, (void *) this) != 0) {
      fprintf(stderr, "%s\n", _device->getLastError());
      return StatusError;
   }

   return StatusGood;
}


// =============================================================================
// End of Player definitions


// -------------------------------------------------------- make_time_string ---
static const char *
make_time_string(const float seconds, const int precision)
{
   static char buf[32];

   int minutes = (int) seconds / 60;
   int secs = (int) seconds % 60;
   if (precision > 0) {
      char  tmp[16], *p;
      float frac = seconds - (int) seconds;
      snprintf(tmp, 16, "%.*f", precision, frac);
      p = tmp + 1;      // skip 0 before decimal point
      snprintf(buf, 32, "%d:%02d%s", minutes, secs, p);
   }
   else
      snprintf(buf, 32, "%d:%02d", minutes, secs);
   return buf;
}


// ------------------------------------------------------------- get_seconds ---
static const float
get_seconds(const char timestr[], TimeFormat *format)
{
   float seconds;
   char  *p, *str;

   str = strdup(timestr);
   if (str == NULL) {
      fprintf(stderr, "get_seconds: can't allocate string buffer.\n");
      exit(EXIT_FAILURE);
   }
   p = strchr(str, ':');
   if (p) {
      float minutes = atof(str);
      *p = '\0';
      p++;        // now str points to minutes str; p points to seconds
      seconds = atof(p);
      seconds += minutes * 60.0;
      *format = TimeFormatMinutesSeconds;
   }
   else
      seconds = atof(str);
   free(str);

   return seconds;
}


// ---------------------------------------- reset_input_mode, set_input_mode ---

static struct termios _saved_term_attributes;

static void
reset_input_mode()
{
   tcsetattr(STDIN_FILENO, TCSANOW, &_saved_term_attributes);
}


static Status
set_input_mode()
{
   struct termios tattr;

   // Make sure stdin is a terminal.
   if (!isatty(STDIN_FILENO)) {
      fprintf(stderr, "Not a terminal.\n");
      return StatusError;
   }

   // Save the terminal attributes so we can restore them at exit.
   tcgetattr(STDIN_FILENO, &_saved_term_attributes);
   atexit(reset_input_mode);

   // Set terminal modes so fast-forward, rewind, etc. will work.
   tcgetattr(STDIN_FILENO, &tattr);
   tattr.c_lflag &= ~(ICANON | ECHO);  // Clear ICANON and ECHO.
   tattr.c_cc[VMIN] = 0;               // so read doesn't block; it
   tattr.c_cc[VTIME] = 0;              //    returns 0 if no input
   tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);

   return StatusGood;
}


// ------------------------------------------------------------------- usage ---

#define USAGE_MSG "\
Usage: %s [options] filename  \n\
       options:  -s NUM    start time                           \n\
                 -e NUM    end time                             \n\
                 -d NUM    duration                             \n\
                 -f NUM    rescale factor                       \n\
                 -c NUM    channel (starting from 0)            \n\
                 -h        view this help screen                \n\
                 -k        disable hotkeys (see below)          \n\
                 -t NUM    time to skip for rewind, fast-forward\n\
                              (default is 4 seconds)            \n\
                 -r        robust - larger audio buffers        \n\
                 -q        quiet - don't print anything         \n\
                 --force   use rescale factor even if peak      \n\
                              amp of float file unknown         \n\
                 -D        audio device name                    \n\
          Notes: -d ignored if you also give -e                 \n\
                 Times can be given as seconds or 0:00.0        \n\
                    If the latter, prints time in same way.     \n\
                 Hotkeys: 'f': fast-forward, 'r': rewind,       \n\
                          'm': mark (print current buffer start time)\n\
                          't': toggle time display              \n\
                          'p': pause / resume                   \n\
                          '-': decrease volume                  \n\
                          '+' or '=': increase volume           \n\
                 To stop playing: cntl-D or cntl-C              \n\
"

void
usage()
{
   fprintf(stderr, USAGE_MSG, PROGNAME);
   exit(EXIT_FAILURE);
}


// -------------------------------------------------------------------- main ---
int
main(int argc, char *argv[])
{
   int         quiet, robust, force, play_chan, hotkeys, requested_bufframes;
   float       start_time, end_time, request_dur, factor, hk_skip_time;
   char        *file_name, *device_name;
   Player      *player;
   Status      status;
   TimeFormat  time_format = TimeFormatSeconds;

   if (argc < 2)
      usage();

   file_name = NULL;
   device_name = DEFAULT_DEVICE_NAME;
   quiet = robust = force = 0;
   play_chan = ALL_CHANS;
   hotkeys = 1;
   start_time = end_time = request_dur = factor = 0.0;
   hk_skip_time = SKIP_SECONDS;

   for (int i = 1; i < argc; i++) {
      char *arg = argv[i];

      if (arg[0] == '-') {
         switch (arg[1]) {
            case 's':               // start time
               if (++i >= argc)
                  usage();
               start_time = get_seconds(argv[i], &time_format);
               break;
            case 'e':               // end time
               if (++i >= argc)
                  usage();
               end_time = get_seconds(argv[i], &time_format);
               break;
            case 'd':               // duration
               if (++i >= argc)
                  usage();
               request_dur = get_seconds(argv[i], &time_format);
               break;
            case 'D':               // audio device name
               if (++i >= argc)
                  usage();
               device_name = argv[i];
               break;
            case 'f':               // rescale factor (for float files)
               if (++i >= argc)
                  usage();
               factor = atof(argv[i]);
               break;
            case 'c':               // channel
               if (++i >= argc)
                  usage();
               play_chan = atoi(argv[i]);
               break;
            case 'k':               // disable hotkeys
               hotkeys = 0;
               break;
            case 't':               // hotkey transport skip time
               if (++i >= argc)
                  usage();
               hk_skip_time = get_seconds(argv[i], &time_format);
               break;
            case 'r':               // robust buffer size
               robust = 1;
               break;
            case 'q':               // no printout
               quiet = 1;
               break;
            case '-':
               if (strcmp(arg, "--force") == 0)
                  force = 1;
               else
                  usage();
               break;
            default:  
               usage();
         }
      }
      else
         file_name = arg;
   }

   // input validation

   if (file_name == NULL) {
      fprintf(stderr, "You didn't give a valid filename.\n");
      exit(EXIT_FAILURE);
   }
   if (start_time < 0.0) {
      fprintf(stderr, "Start time must be positive.\n");
      exit(EXIT_FAILURE);
   }
   if (start_time > 0.0 && end_time > 0.0 && start_time >= end_time) {
      fprintf(stderr, "Start time must be less than end time.\n");
      exit(EXIT_FAILURE);
   }
   if (end_time == 0.0 && request_dur > 0.0)
      end_time = start_time + request_dur;
   if (factor < 0.0) {
      fprintf(stderr, "Rescale factor must be zero or greater.\n");
      exit(EXIT_FAILURE);
   }
   if (hotkeys && hk_skip_time <= 0.0) {
      fprintf(stderr, "Skip time must be greater than zero.\n");
      exit(EXIT_FAILURE);
   }

   requested_bufframes = BUF_FRAMES;
   if (robust) {
      requested_bufframes *= ROBUST_FACTOR;
      if (!quiet)
         printf("Warning: \"robust\" mode causes second count to run ahead.\n");
   }

   // Set up terminal to handle hotkey input.
   if (hotkeys) {
      status = set_input_mode();
      if (status != StatusGood)
         exit(EXIT_FAILURE);
   }

   player = new Player(file_name, device_name, start_time, end_time, play_chan,
               requested_bufframes, factor, force, hotkeys, hk_skip_time, quiet,
               time_format);

   status = player->configure();
   if (status != StatusGood)
      exit(EXIT_FAILURE);

   status = player->play();
   if (status == StatusGood) {
      while (audioDone == false)
         sleep(1);
   }
   else
      exit(EXIT_FAILURE);

   if (!quiet)
      printf("\n");

   delete player;

   return EXIT_SUCCESS;
}

#endif /* #if defined(LINUX) || defined(MACOSX) */
