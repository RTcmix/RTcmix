// audio_devices.h

#ifdef __cplusplus
extern "C" {
#endif

int create_audio_devices(int recordAndPlay, int chans, float sr, int *buffersize);

int create_audio_file_device(const char *outfilename,
							 int header_type,
							 int sample_format,
							 int chans,
							 float srate,
							 int normalize_output_floats,
							 int check_peaks,
							 int play_audio_too);

int audio_input_is_initialized();

void destroy_audio_devices();

int destroy_audio_file_device();

#ifdef __cplusplus
}
#endif
