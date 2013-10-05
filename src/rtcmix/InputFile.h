//
//  InputFile.h
//
//  Created by Douglas Scott on 9/24/11.
//

#ifndef INPUTDESC_H
#define INPUTDESC_H

#include "Lockable.h"
#include "rt_types.h"
#include <sys/types.h>
#include <string.h>

typedef int (*ReadFun)(int,int,int,off_t,long,BufPtr,int,int,const short[],short,void*);

/* definition of input file struct used by rtinput */
struct InputFile : public Lockable {
public:
	enum Type { FileType = 0, AudioDeviceType = 1, InMemoryType = 2 };
#ifdef MULTI_THREAD
	static void createConversionBuffers(int inBufSamps);
	static void destroyConversionBuffers();
#endif
	InputFile();
    void init(int inFd, const char *inFileName, Type inType, int inHeaderType,
              int inDataFormat, int inDataLocation, long nFrames, float inSampleRate,
              int inChannels, double inDuration);
	
    void reference();
    void unreference();
	
	int	getFD() const { return _fd; }
	const char *fileName() const { return _filename; }
	bool hasFile(const char *inPath) const { return strcmp(inPath, _filename) == 0; }
	float sampleRate() const { return _srate; }
	short channels() const { return _chans; }
	bool isAudioDevice() const { return _fileType == AudioDeviceType; }
	short dataFormat() const { return _data_format; }
	int dataLocation() const { return _data_location; }
	double duration() const { return _dur; }
	
    off_t readSamps(off_t     cur_offset,       /* current file position before read */
                  BufPtr      dest,             /* interleaved buffer from inst */
                  int         dest_chans,       /* number of chans interleaved */
                  int         dest_frames,      /* frames in interleaved buffer */
                  const short src_chan_list[],  /* list of in-bus chan numbers from inst */
                  /* (or NULL to fill all chans) */
                  short       src_chans         /* number of in-bus chans to copy */
    );
	bool isOpen() const { return _fd > 0; }
    
protected:
	int  loadSamps(long inFrames);
	off_t copySamps(off_t     cur_offset,       /* current file position before read */
				BufPtr      dest,             /* interleaved buffer from inst */
				int         dest_chans,       /* number of chans interleaved */
				int         dest_frames,      /* frames in interleaved buffer */
				const short src_chan_list[],  /* list of in-bus chan numbers from inst */
				/* (or NULL to fill all chans) */
				short       src_chans         /* number of in-bus chans to copy */
    );

	void close();

private:
	char     *_filename;         /* allocated by rtinput() */
	int      _fd;                /* file descriptor, or NO_FD, or AUDIO_DEVICE */
	Type     _fileType;         /* FileType, AudioDeviceType, InMemoryType */
	short    _header_type;       /* e.g., AIFF_sound_file (in sndlib.h) */
	short    _data_format;       /* e.g., snd_16_linear (in sndlib.h) */
	short    _is_float_format;   /* true if data format is 32-bit float */
	int      _data_location;     /* offset of sound data start in file */
	long     _endbyte;           /* index of byte following last sample */
	float    _srate;
	short    _chans;
	double   _dur;
    void *	 _readBuffer;
    BufPtr 	 _memBuffer;
	int      _refcount;
    ReadFun  _readFunction;
	static const int	sScratchBufferSize = 4096;
	static char			sScratchBuffer[];
#ifdef MULTI_THREAD
	static char *		sConversionBuffers[];
#endif
};

#endif
