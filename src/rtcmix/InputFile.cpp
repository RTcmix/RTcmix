//
//  InputFile.cpp
//
//  Created by Douglas Scott on 9/24/11.
//

#include "InputFile.h"
#include "RTcmix.h"
#include <sndlib.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sndlibsupport.h>
#include <ugens.h>
#include "byte_routines.h"
#ifdef MULTI_THREAD
#include "RTThread.h"
#endif

#undef FILE_DEBUG

/* The define below is to disable some fancy bus-mapping code for file
 input that was not well thought out. As a user, when I open a file,
 I typically just want the instrument to read all its channels without
 me worrying how many there are. If the instrument can read only one,
 I'll tell it which one in its inchan pfield. But what numbers do I use
 in my call to bus_config for the "in" bus? As a user, I don't care.
 So we're ignoring them here, and making sure that inst->inputchans
 is set to match the number of chans in the file (in rtsetinput).
 
 What we lose by ignoring the bus numbers is consistency of the
 bus_config interface between file and rt input. We also lose the
 ability to select arbitrary channels from a file to send into an
 instrument. (But what instruments could make use of this?) My
 feeling is that this selection feature would be more trouble for
 the user than it's worth. What do you think?
 
 If we expand the rtinput syntax to allow opening more than one
 file for an instrument to use, then this could all change. But
 it's not easy to see how, exactly. Note that the commented out
 code below interprets the bus numbers as a selection of channels
 from the input file. However, if you could say:
 
 rtinput("foo.snd", "in 0-1"); rtinput("bar.snd", "in 2-3")
 
 Then you'd expect this to work the way an aux-in bus does now: the
 first file is read into buses 0-1, the second into buses 2-3. But
 these "buses" aren't really buses the way aux buses are. Instead,
 they're just private buffers in this file (or in an instrument,
 depending on how you look at it). This makes sense, because sound
 files aren't real-time input, and instruments often don't read from
 the same section of a file during the same timeslice -- so they'd
 hardly ever be able to share a file input bus. So with the rtinput
 syntax above, we'd just pretend that those are real buses. What
 kind of bus_config would work with those two rtinputs, and what
 would its in-bus numbers mean?
 
 JGG, 27-May-00
 */
#define IGNORE_BUS_COUNT_FOR_FILE_INPUT

static inline long lmin(long a, long b) { return a < b ? a : b; }
static inline long lmax(long a, long b) { return a > b ? a : b; }


/* ----------------------------------------------------- read_float_samps --- */
static int
read_float_samps(
      int         fd,               /* file descriptor for open input file */
      int         data_format,      /* sndlib data format of input file */
      int         file_chans,       /* total chans in input file */
      off_t       cur_offset,       /* current file position before read */
      long        endbyte,          /* first byte following last file sample */
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      const short src_chan_list[],  /* list of in-bus chan numbers from inst */
                 /* (or NULL to fill all chans) */
      short       src_chans,         /* number of in-bus chans to copy */
      void *      read_buffer        /* block to read from disk into */
      )
{    
    const int bytes_per_samp = sizeof(float);
    char *bufp = (char *) read_buffer;
    
    const int bytes_requested = dest_frames * file_chans * bytes_per_samp;
    const long bytes_remaining = endbyte - cur_offset;
    const long extra_bytes = (bytes_requested > bytes_remaining)
    ? bytes_requested - bytes_remaining : 0;
    ssize_t bytes_to_read = lmin(bytes_remaining, bytes_requested);
    
    while (bytes_to_read > 0) {
        ssize_t bytes_read = read(fd, bufp, bytes_to_read);
        if (bytes_read == -1) {
            perror("read_float_samps (read)");
            RTExit(1);
        }
        if (bytes_read == 0)          /* EOF */
            break;
        
        bufp += bytes_read;
        bytes_to_read -= bytes_read;
    }
    
    /* If we reached EOF, zero out remaining part of buffer that we
     expected to fill.
     */
    bytes_to_read += extra_bytes;
    while (bytes_to_read > 0) {
        (* (float *) bufp) = 0.0;
        bufp += bytes_per_samp;
        bytes_to_read -= bytes_per_samp;
    }
    
    /* Copy interleaved file buffer to dest buffer, with bus mapping. */
    
    const int src_samps = dest_frames * file_chans;
    float *fbuf = (float *) read_buffer;
    
    for (int n = 0; n < dest_chans; n++) {
#ifdef IGNORE_BUS_COUNT_FOR_FILE_INPUT
        const int chan = n;
#else
        const int chan = src_chan_list ? src_chan_list[n] : n;
#endif
        int j = n;
        for (int i = chan; i < src_samps; i += file_chans, j += dest_chans)
            dest[j] = (BUFTYPE) fbuf[i];
    }
    
#if MUS_LITTLE_ENDIAN
    const bool swap = IS_BIG_ENDIAN_FORMAT(data_format);
#else
    const bool swap = IS_LITTLE_ENDIAN_FORMAT(data_format);
#endif
    if (swap) {
        for (int i = 0; i < src_samps; i++)
            byte_reverse4(&dest[i]);
    }
    
    return 0;
}


/* ----------------------------------------------------- read_24bit_samps --- */
static int
read_24bit_samps(
      int         fd,               /* file descriptor for open input file */
      int         data_format,      /* sndlib data format of input file */
      int         file_chans,       /* total chans in input file */
      off_t       cur_offset,       /* current file position before read */
      long        endbyte,          /* first byte following last file sample */
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      const short src_chan_list[],  /* list of in-bus chan numbers from inst */
                 /* (or NULL to fill all chans) */
      short       src_chans,         /* number of in-bus chans to copy */
      void *      read_buffer        /* block to read from disk into */
      )
{
    
    const int bytes_per_samp = 3;         /* 24-bit int */
    unsigned char *bufp = (unsigned char *) read_buffer;
    
    const int bytes_requested = dest_frames * file_chans * bytes_per_samp;
    const long bytes_remaining = endbyte - cur_offset;
    const long extra_bytes = (bytes_requested > bytes_remaining)
    ? bytes_requested - bytes_remaining : 0;
    ssize_t bytes_to_read = lmin(bytes_remaining, bytes_requested);
    
    while (bytes_to_read > 0) {
        ssize_t bytes_read = read(fd, bufp, bytes_to_read);
        if (bytes_read == -1) {
            perror("read_24bit_samps (read)");
            RTExit(1);
        }
        if (bytes_read == 0)          /* EOF */
            break;
        
        bufp += bytes_read;
        bytes_to_read -= bytes_read;
    }
    
    /* If we reached EOF, zero out remaining part of buffer that we
     expected to fill.
     */
    bytes_to_read += extra_bytes;
    while (bytes_to_read > 0) {
        *bufp++ = 0;
        bytes_to_read--;
    }
    
    /* Copy interleaved file buffer to dest buffer, with bus mapping. */
    
    const int src_samps = dest_frames * file_chans;
    const BUFTYPE scaleFactor = 1 / (BUFTYPE) (1 << 8);
    unsigned char *cbuf = (unsigned char *) read_buffer;
    
    for (int n = 0; n < dest_chans; n++) {
#ifdef IGNORE_BUS_COUNT_FOR_FILE_INPUT
        const int chan = n;
#else
        const int chan = src_chan_list ? src_chan_list[n] : n;
#endif
        int i = chan * bytes_per_samp;
        const int incr = file_chans * bytes_per_samp;
        const int src_bytes = src_samps * bytes_per_samp;
        if (data_format == MUS_L24INT) {
            for (int j = n; i < src_bytes; i += incr, j += dest_chans) {
                int samp = (int) (((cbuf[i + 2] << 24)
                                   + (cbuf[i + 1] << 16)
                                   + (cbuf[i] << 8)) >> 8);
                dest[j] = (BUFTYPE) samp * scaleFactor;
            }
        }
        else {   /* data_format == MUS_B24INT */
            for (int j = n; i < src_bytes; i += incr, j += dest_chans) {
                int samp = (int) (((cbuf[i] << 24)
                                   + (cbuf[i + 1] << 16)
                                   + (cbuf[i + 2] << 8)) >> 8);
                dest[j] = (BUFTYPE) samp * scaleFactor;
            }
        }
    }
    
    return 0;
}


/* ----------------------------------------------------- read_short_samps --- */
static int
read_short_samps(
      int         fd,               /* file descriptor for open input file */
      int         data_format,      /* sndlib data format of input file */
      int         file_chans,       /* total chans in input file */
      off_t       cur_offset,       /* current file position before read */
      long        endbyte,          /* first byte following last file sample */
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      const short src_chan_list[],  /* list of in-bus chan numbers from inst */
                 /* (or NULL to fill all chans) */
      short       src_chans,         /* number of in-bus chans to copy */
      void *      read_buffer        /* block to read from disk into */
      )
{
    const int bytes_per_samp = 2;         /* short int */
    char *bufp = (char *) read_buffer;
    
    const int bytes_requested = dest_frames * file_chans * bytes_per_samp;
    const long bytes_remaining = endbyte - cur_offset;
    const long extra_bytes = (bytes_requested > bytes_remaining)
    ? bytes_requested - bytes_remaining : 0;
    ssize_t bytes_to_read = lmin(bytes_remaining, bytes_requested);
    
    while (bytes_to_read > 0) {
        ssize_t bytes_read = read(fd, bufp, bytes_to_read);
        if (bytes_read == -1) {
            perror("read_short_samps (read)");
            RTExit(1);
        }
        if (bytes_read == 0)          /* EOF */
            break;
        
        bufp += bytes_read;
        bytes_to_read -= bytes_read;
    }
    
    /* If we reached EOF, zero out remaining part of buffer that we
     expected to fill.
     */
    bytes_to_read += extra_bytes;
    while (bytes_to_read > 0) {
        (* (short *) bufp) = 0;
        bufp += bytes_per_samp;
        bytes_to_read -= bytes_per_samp;
    }
    
    /* Copy interleaved file buffer to dest buffer, with bus mapping. */
    
#if MUS_LITTLE_ENDIAN
    const bool swap = IS_BIG_ENDIAN_FORMAT(data_format);
#else
    const bool swap = IS_LITTLE_ENDIAN_FORMAT(data_format);
#endif
    
    const int src_samps = dest_frames * file_chans;
    short *sbuf = (short *)read_buffer;
    
    for (int n = 0; n < dest_chans; n++) {
#ifdef IGNORE_BUS_COUNT_FOR_FILE_INPUT
        const int chan = n;
#else
        const int chan = src_chan_list ? src_chan_list[n] : n;
#endif
        if (swap) {
            int j = n;
            for (int i = chan; i < src_samps; i += file_chans, j += dest_chans) {
                sbuf[i] = reverse_int2(&sbuf[i]);
                dest[j] = (BUFTYPE) sbuf[i];
            }
        }
        else {
            int j = n;
            for (int i = chan; i < src_samps; i += file_chans, j += dest_chans)
                dest[j] = (BUFTYPE) sbuf[i];
        }
    }
    
    return 0;
}

char InputFile::sScratchBuffer[sScratchBufferSize];

#ifdef MULTI_THREAD

char *		InputFile::sConversionBuffers[RT_THREAD_COUNT];

void InputFile::createConversionBuffers(int inBufSamps)
{
	/* Allocate buffers needed to convert input audio files as they are read */
	for (int i = 0; i < RT_THREAD_COUNT; ++i) {
		sConversionBuffers[i] = (char *) malloc(sizeof(BUFTYPE) * MAXCHANS * inBufSamps);
	}
}

void InputFile::destroyConversionBuffers()
{
	for (int i = 0; i < RT_THREAD_COUNT; ++i) {
		free(sConversionBuffers[i]);
		sConversionBuffers[i] = NULL;
	}
}

#endif

InputFile::InputFile() : _filename(NULL), _fd(NO_FD), _readBuffer(NULL), _memBuffer(NULL), _gainScale(1.0f)
{
}

InputFile::~InputFile()
{
#ifdef FILE_DEBUG
	rtcmix_debug(NULL, "InputFile::~InputFile");
#endif
	if (_refcount > 0) {
		rtcmix_debug(NULL, "Input file '%s' had %d outstanding references when destroyed", _filename, _refcount);
	}
	close();
}

void InputFile::init(int inFd, const char *inFileName, Type inType, int inHeaderType,
          int inDataFormat, int inDataLocation, long inFrames, float inSampleRate,
          int inChannels, double inDuration)
{
    _filename = strdup(inFileName);
    _fd = inFd;
    _refcount = 0;
    _fileType = inType;
    _header_type = inHeaderType;
    _data_format = inDataFormat;
    _is_float_format = IS_FLOAT_FORMAT(_data_format);
    _data_location = inDataLocation;
    _srate = inSampleRate;
    _chans = inChannels;
    _dur = inDuration;
	
    int bytes_per_samp = ::mus_data_format_to_bytes_per_sample(_data_format);

    assert(_chans <= MAXCHANS);

	if (_is_float_format)
		_readFunction = &read_float_samps;
	else if (IS_24BIT_FORMAT(_data_format))
		_readFunction = &read_24bit_samps;
	else
		_readFunction = &read_short_samps;

#ifndef MULTI_THREAD
	_readBuffer = (void *) malloc((size_t) RTcmix::bufsamps() * MAXCHANS * bytes_per_samp);
	if (_readBuffer == NULL) {
		die("rtinput", "Unable to allocate read buffer for input file");
		RTExit(1);
	}
#endif

	if (_fileType == InMemoryType) {
		_endbyte = inFrames * bytes_per_samp * _chans;
    	if (loadSamps(inFrames) != 0) {
			rtcmix_warn("rtinput", "File '%s' cannot be loaded into memory -- defaulting to regular load");
			_fileType = FileType;
			_endbyte += _data_location;
		}
	}
	else
		_endbyte = _data_location + (inFrames * bytes_per_samp * _chans);
}

#define MM_IN_GAIN_FACTOR 32767.0f // goose it up for RTcmix

void InputFile::init(BufPtr inBuffer, const char *inBufferName, long inFrames, float inSampleRate, int inChannels)
{
	rtcmix_debug("InputFile::init", "inBuffer %p, frames %ld, chans %d", inBuffer, inFrames, inChannels);
    _filename = strdup(inBufferName);
    _fd = USE_MM_BUF;
    _refcount = 0;
    _fileType = InMemoryType;
    _header_type = 0;
    _data_format = NATIVE_FLOAT_FMT;
    _is_float_format = 1;
    _data_location = 0;
    _srate = inSampleRate;
    _chans = inChannels;
    _dur = inFrames/inSampleRate;
	
    int bytes_per_samp = ::mus_data_format_to_bytes_per_sample(_data_format);
	
    assert(_chans <= MAXCHANS);
	
	_readFunction = &read_float_samps;
	_endbyte = inFrames * bytes_per_samp * _chans;

#ifndef OPENFRAMEWORKS
	_gainScale = MM_IN_GAIN_FACTOR;
#endif
	_memBuffer = inBuffer;
}

// TODO: DAS: This needs a lock to prevent modification during read

void InputFile::reinit(BufPtr inBuffer, long inFrames, int inChannels)
{
    _chans = inChannels;
    _dur = inFrames/_srate;
	
	rtcmix_debug("InputFile::reinit", "inBuffer %p, frames %ld, chans %d", inBuffer, inFrames, inChannels);
    int bytes_per_samp = ::mus_data_format_to_bytes_per_sample(_data_format);
	
    assert(_chans <= MAXCHANS);
	
	_endbyte = inFrames * bytes_per_samp * _chans;
	
	_memBuffer = inBuffer;
}

void InputFile::reference()
{
	if (++_refcount == 1) {
		// In here we can do any post-initialization that only needs to be done (once) when we are
		// sure that this InputFile is being used by an instrument.
	}
#ifdef FILE_DEBUG
	rtcmix_debug(NULL, "InputFile::reference: refcount = %d\n", _refcount);
#endif
}

void InputFile::unreference()
{
#ifdef FILE_DEBUG
	rtcmix_debug(NULL, "InputFile::unreference: refcount = %d\n", _refcount);
#endif
	if (--_refcount <= 0) {
		close();
	}
}

void InputFile::close()
{
	if (_fd != USE_MM_BUF) {	// MM buffers are not owned by us
		if (_memBuffer) {
#ifdef FILE_DEBUG
			rtcmix_debug(NULL, "\tInputFile::close: freeing _memBuffer");
#endif
			free(_memBuffer);
			_memBuffer = NULL;
		}
		if (_fd > 0) {
#ifdef FILE_DEBUG
			rtcmix_debug(NULL, "\tInputFile::close: closing fd %d", _fd);
#endif
			mus_file_close(_fd);
			_fd = NO_FD;
		}
	}
#ifndef MULTI_THREAD
	if (_readBuffer)
		free(_readBuffer);
	_readBuffer = NULL;
#endif
	if (_filename)
		free(_filename);
	_filename = NULL;
	_header_type = MUS_UNSUPPORTED;
	_data_format = MUS_UNSUPPORTED;
	_is_float_format = 0;
	_data_location = 0;
	_endbyte = 0;
	_srate = 0.0;
	_chans = 0;
	_dur = 0.0;
}

off_t InputFile::readSamps(off_t cur_offset,
                         BufPtr dest,
                         int dest_chans,
                         int dest_frames,
                         const short src_chan_list[],
                         short src_chans)
{
#ifndef IGNORE_BUS_COUNT_FOR_FILE_INPUT
    assert(dest_chans >= src_chans);
#endif
    assert(_chans >= dest_chans);
	if (_fileType == InMemoryType) {
		(void)copySamps(cur_offset, dest, dest_chans, dest_frames, src_chan_list, src_chans);
	}
	else {
#ifdef MULTI_THREAD
		_readBuffer = sConversionBuffers[RTThread::GetIndexForThread()];
#endif
		{
			AutoLock fileLock(this);
			if (lseek(_fd, cur_offset, SEEK_SET) == -1) {
				perror("RTcmix::readFromInputFile (lseek)");
				RTExit(1);
			}
			
			int status = (*this->_readFunction)(_fd,
										 _data_format,
										 _chans,
										 cur_offset,
										 _endbyte,
										 dest, dest_chans, dest_frames,
										 src_chan_list, src_chans,
										 _readBuffer);
		}
	}
    int bytes_per_samp = ::mus_data_format_to_bytes_per_sample(_data_format);
    return dest_frames * _chans * bytes_per_samp;
}

int InputFile::loadSamps(long inFrames)
{
#ifdef FILE_DEBUG
	rtcmix_debug(NULL, "\tInputFile::loadSamps: allocating _memBuffer for %lu bytes", (size_t)inFrames * _chans * sizeof(BUFTYPE));
#endif
	_memBuffer = (BufPtr) calloc((size_t) inFrames * _chans, sizeof(BUFTYPE));
	if (_memBuffer == NULL) {
		perror("malloc");
		return -1;
	}
	if (lseek(_fd, _data_location, SEEK_SET) == -1) {
		perror("RTcmix::readFromInputFile (lseek)");
		RTExit(1);
	}
	const short src_chan_list[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	long framesRead = 0;
	long bytesRead = 0;
	int status = 0;
    const int bytesPerFrame = ::mus_data_format_to_bytes_per_sample(_data_format) * _chans;
	const int framesPerRead = sScratchBufferSize / bytesPerFrame;
	while (framesRead < inFrames) {
		long frameCount = inFrames - framesRead;
		if (frameCount > framesPerRead)
			frameCount = framesPerRead;
		const long byteCount = bytesPerFrame * frameCount;
//		printf("reading %d frames (%d-channel) at offset %d via scratch buffer, into _memBuffer[%d*%d]\n", frameCount, _chans, lseek(_fd, 0, SEEK_CUR), framesRead, _chans);
		status = (*this->_readFunction)(_fd,
										_data_format,
										_chans,
										_data_location + bytesRead,
										_endbyte,
										&_memBuffer[framesRead*_chans], _chans, (int)frameCount,
										src_chan_list, _chans,
										sScratchBuffer);
		if (status != 0)
			break;
		framesRead += frameCount;
		bytesRead += byteCount;
	}
	return status;
}

off_t InputFile::copySamps(off_t     cur_offset,       /* current file position - used to offset into cached waveform */
				BufPtr      dest,             /* interleaved buffer from inst */
				int         dest_chans,       /* number of chans interleaved */
				int         dest_frames,      /* frames in interleaved buffer */
				const short src_chan_list[],  /* list of in-bus chan numbers from inst */
				/* (or NULL to fill all chans) */
				short       src_chans         /* number of in-bus chans to copy */
)
{
    const int bytes_per_samp = sizeof(float);
	const int bytes_per_frame = bytes_per_samp * _chans;
	// Original file values
    off_t fileAudioOffset = (cur_offset - _data_location);
    const long fileBytesRemaining = long(_endbyte - cur_offset);
	// Convert byte offset into original file into an offset into (float) _memBuffer
	long membufOffset = long(fileAudioOffset / ::mus_data_format_to_bytes_per_sample(_data_format));
    const int bufBytesRequested = dest_frames * _chans * bytes_per_samp;
	const long bufBytesRemaining = (fileBytesRemaining / ::mus_data_format_to_bytes_per_sample(_data_format)) * bytes_per_frame;
    const long extra_bytes = (bufBytesRequested > bufBytesRemaining) ? bufBytesRequested - bufBytesRemaining : 0;
    ssize_t bytes_to_copy = lmin(bufBytesRemaining, bufBytesRequested);
    
	if (dest_chans == _chans && _gainScale == 1.0f
#ifndef IGNORE_BUS_COUNT_FOR_FILE_INPUT
		&& !src_chan_list
#endif
		) {
		
		if (bytes_to_copy > 0) {
//			printf("memcpy'ing %d bytes (%d frames) to dest from _memBuffer[%d]\n", bytes_to_copy, bytes_to_copy/(_chans * bytes_per_samp), membufOffset);
			memcpy(dest, &_memBuffer[membufOffset], bytes_to_copy);
			bytes_to_copy = 0;
		}
		
		/* If we reached EOF, zero out remaining part of buffer that we
		 expected to fill.
		 */
		long bytesToZero = bytes_to_copy + extra_bytes;
		if (bytesToZero > 0) {
			long offset = lmax(bytes_to_copy, 0L);
//			printf("zeroing dest buffer at offset %d, %d bytes worth\n", offset, bytesToZero);
			memset((char *)dest + offset, 0, bytesToZero);
		}
	}
    else {
		/* Copy interleaved _memBuffer to dest buffer, with bus mapping and/or gain adjust. */
		
		const int src_samps = dest_frames * _chans;
		BufPtr buf = &_memBuffer[membufOffset];
//		printf("copying %d samps (%d %d-channel frames) to dest from _memBuffer[%d]\n", src_samps, src_samps/_chans, _chans, membufOffset);
		
		for (int n = 0; n < dest_chans; n++) {
#ifdef IGNORE_BUS_COUNT_FOR_FILE_INPUT
			const int chan = n;
#else
			const int chan = src_chan_list ? src_chan_list[n] : n;
#endif
			int j = n;
			for (int i = chan; i < src_samps; i += _chans, j += dest_chans)
				dest[j] = buf[i] * _gainScale;
		}
	}
    return 0;
}
