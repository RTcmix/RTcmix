// audiostream.h
//
// Template inlines and classes used to generate the audio format conversion
// routines for AudioDeviceImpl.cpp
//

enum Endian { Little, Big };

inline unsigned short swapit(unsigned short us) { return (((us >> 8) & 0xff) | ((us & 0xff) << 8)); }

inline short swapit(short s) { return swapit((unsigned short) s); }

inline unsigned swapit(unsigned ul) {
    return (ul >> 24) | ((ul >> 8) & 0xff00) | ((ul << 8) & 0xff0000) | (ul << 24);
}

inline int swapit(int x) { return swapit((unsigned)x); }

inline float swapit(float uf) {
    union { unsigned l; float f; } u;
    u.f = uf;
    u.l = swapit(u.l);
    return (u.f);
}

template<class SampleType>
inline SampleType swap(bool doSwap, SampleType value);

template<>
inline short swap<short>(bool doSwap, short value) { return doSwap ? swapit(value) : value; }
 
template<>
inline float swap<float>(bool doSwap, float value) { return doSwap ? swapit(value) : value; }
 
 
template< class SampleType >
inline SampleType deNormalize(bool doIt, const SampleType &value);
 
template<>
inline short deNormalize<short>(bool doIt, const short &value) { return value; }
 
template<>
inline float deNormalize<float>(bool doIt, const float &value) { return doIt ? value * 32767.0f : value; }


const static float kNormalizer = 1/32768.0;

template< class SampleType >
inline SampleType normalize(bool doIt, const SampleType &value);
 
template<>
inline short normalize<short>(bool doIt, const short &value) { return value; }
 
template<>
inline float normalize<float>(bool doIt, const float &value) { return doIt ? value * kNormalizer : value; }

#ifdef LINUX
static const Endian kMachineEndian = Little;
#else
static const Endian kMachineEndian = Big;
#endif

template < class SampleType, Endian theEndian, bool isNormalized = false >
class InterleavedStream {
public:
	static const Endian endian = theEndian;
	static const bool normalized = isNormalized;
	typedef SampleType StreamType;
	typedef StreamType ChannelType;
	static int channelIncrement(int chans) { return chans; }
	static ChannelType *innerFromOuter(StreamType *streamData, int ch) { return &streamData[ch]; }
};

template < class SampleType, Endian theEndian, bool isNormalized = false >
class NonInterleavedStream {
public:
	static const Endian endian = theEndian;
	static const bool normalized = isNormalized;
	typedef SampleType *StreamType;
	typedef SampleType ChannelType;
	static int channelIncrement(int chans) { return 1; }
	static ChannelType *innerFromOuter(StreamType *streamData, int ch) { return streamData[ch]; }
};

