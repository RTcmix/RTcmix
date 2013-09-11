//
//  RawDataFile.h
//  RTcmix
//
//  Created by Douglas Scott on 4/26/13.
//
//

#ifndef __RTCMIX_RAWDATAFILE_H__
#define __RTCMIX_RAWDATAFILE_H__

#include <stdint.h>
#include <stdio.h>

class RawDataFile {
public:
	RawDataFile(const char *fileName);
	virtual ~RawDataFile();
	
	int openFileWrite(const bool clobber);
	int openFileRead();
	int closeFile();
	template <typename T>
	inline int read(T *val, int count=1) { return _read(val, count); }
	
protected:
	// Return 0 if valid read; -1 if error.
	template <typename T>
	inline int _write(const T val)
	{
		long nitems = fwrite(&val, sizeof(T), 1, _stream);
		if (nitems != 1 && ferror(_stream))
			return -1;
		return 0;
	}
	
	// Return 0 if valid read; -1 if error or EOF.
	template <typename T>
	inline int _read(T *val, int count=1)
	{
		long nitems = fread(val, sizeof(T), count, _stream);
		return (nitems != count) ? -1 : 0;
	}
	
	// swapping functions taken from src/audio/audiostream.h
	
	inline uint16_t _swapit(uint16_t us) {
		return ((us >> 8) & 0xff) | ((us & 0xff) << 8);
	}
	inline int16_t _swapit(int16_t s) { return _swapit((uint16_t) s); }
	
	inline uint32_t _swapit(uint32_t ul) {
		return (ul >> 24) | ((ul >> 8) & 0xff00)
		| ((ul << 8) & 0xff0000) | (ul << 24);
	}
	inline int32_t _swapit(int32_t l) { return _swapit((uint32_t) l); }
	
	inline uint64_t _swapit(uint64_t ull) {
		return (((uint64_t) _swapit((uint32_t) (ull >> 32))) << 32)
		| (uint64_t) _swapit((uint32_t) (ull & 0xffffffff));
	}
	inline int64_t _swapit(int64_t ll) { return _swapit((uint64_t) ll); }
	
	// XXX Do these float and double swappers really work cross-platform??
	
	inline float _swapit(float x) {
		union { uint32_t l; float f; } u;
		u.f = x;
		u.l = _swapit(u.l);
		return u.f;
	}
	
	inline double _swapit(double x) {
		union { uint64_t ll; double d; } u;
		u.d = x;
		u.ll = _swapit(u.ll);
		return u.d;
	}
	
	char *_filename;
	FILE *_stream;
	bool _swap;
};

#endif /* defined(__RTCMIX_RAWDATAFILE_H__) */
