// audio_devices.h

#ifdef __cplusplus
// From audio_dev_creator.cpp.  Only used by C++ sources.
AudioDevice *createAudioDevice(const char *inputDesc, const char *outputDesc, bool recording, bool playing);
extern "C" {
#endif

int create_audio_devices(int record, int play, int chans, float sr, int *buffersize, int numBuffers);

int create_audio_file_device(const char *outfilename,
							 int header_type,
							 int sample_format,
							 int chans,
							 float srate,
							 int normalize_output_floats,
							 int check_peaks);

int audio_input_is_initialized();

int destroy_audio_file_device();

#ifdef __cplusplus
}
#endif
