#ifndef SNDLIB_H
#define SNDLIB_H


/* taken from libtool's demo/foo.h to try to protect us from C++ and ancient C's */
#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

#undef __P
#if defined (__STDC__) || defined (_AIX) || (defined (__mips) && defined (_SYSTYPE_SVR4)) || defined(WIN32) || defined(__cplusplus)
# define __P(protos) protos
#else
# define __P(protos) ()
#endif


#define SNDLIB_VERSION 5
#define SNDLIB_REVISION 5

/* 1: Oct-98 */
/* 2: Oct-98: removed header override functions */
/* 3: Dec-98: removed output_scaler */
/* 4: Jan-99: Sun-related word-alignment changes, C++ fixups */
/* 5: Mar-99: changed float_sound to omit the scaling by SNDLIB_SNDFLT, Apr-99: removed perror calls */


/* try to figure out what type of machine (and in worst case, what OS) we're running on */
/* gcc has various compile-time macros like #cpu, but we're hoping to run in Metroworks C, Watcom C, MSC, CodeWarrior, MPW, etc */

#if defined(HAVE_CONFIG_H)
  #include "config.h"
  #if (!defined(WORDS_BIGENDIAN))
     #define SNDLIB_LITTLE_ENDIAN 3
  #endif
  #if (SIZEOF_INT_P != SIZEOF_INT)
     #define LONG_INT_P 1
  #else 
     #define LONG_INT_P 0
  #endif
#else
  #if defined(ALPHA)
     #define LONG_INT_P 1
  #else 
     #define LONG_INT_P 0
  #endif
  #define RETSIGTYPE void
  #ifdef __LITTLE_ENDIAN__
    /* NeXTStep on Intel */
    #define SNDLIB_LITTLE_ENDIAN 1
  #else
    #ifdef BYTE_ORDER
      #if (BYTE_ORDER == LITTLE_ENDIAN)
        /* SGI possibility (/usr/include/sys/endian.h), and Linux (/usr/include/bytesex.h and endian.h) */
        /* Alpha is apparently /usr/include/alpha/endian.h */
        #define SNDLIB_LITTLE_ENDIAN 2
      #endif
    #endif
  #endif
#endif

#if defined(ALPHA) || defined(WINDOZE)
  #define SNDLIB_LITTLE_ENDIAN 3
#endif

#if (!(defined(MACOS))) && (defined(MPW_C) || defined(macintosh) || defined(__MRC__))
  #define MACOS 1
#endif

/* due to project builder stupidity, we can't always depend on -D flags here (maybe we need a SNDLIB_OS macro?) */
/* these wouldn't work with autoconf anyway, so we'll do it by hand */

#if (!defined(SGI)) && (!defined(NEXT)) && (!defined(LINUX)) && (!defined(MACOS)) && (!defined(BEOS)) && (!defined(SUN)) && (!defined(UW2)) && (!defined(SCO5)) && (!defined(ALPHA)) && (!defined(WINDOZE))
  #if defined(__dest_os)
    /* we're in Metrowerks Land */
    #if (__dest_os == __be_os)
      #define BEOS 1
    #else
      #if (__dest_os == __mac_os)
        #define MACOS 1
      #endif
    #endif
  #else
    #if macintosh
      #define MACOS 1
    #else
      #if (__WINDOWS__) || (__NT__) || (_WIN32) || (__CYGWIN__)
        #define WINDOZE 1
        #define SNDLIB_LITTLE_ENDIAN 3
      #else
        #ifdef __alpha__
          #define ALPHA 1
          #define SNDLIB_LITTLE_ENDIAN 3
        #endif
      #endif
    #endif
  #endif
#endif  

/* others apparently are __QNX__ __bsdi__ __FreeBSD__ */

#if defined(LINUX) && defined(PPC) && (!(defined(MKLINUX)))
  #define MKLINUX 1
#endif

#if defined(MKLINUX) || defined(LINUX) || defined(SCO5) || defined(UW2) || defined(HAVE_SOUNDCARD_H) || defined(HAVE_SYS_SOUNDCARD_H) || defined(HAVE_MACHINE_SOUNDCARD_H)
  #define HAVE_OSS 1
#else
  #define HAVE_OSS 0
#endif

#ifdef SNDLIB_LITTLE_ENDIAN
  #define COMPATIBLE_FORMAT snd_16_linear_little_endian
#else
  #define COMPATIBLE_FORMAT snd_16_linear
#endif

/* M_PI is more usual */
#if (defined(HAVE_CONFIG_H) && (!defined(HAVE_PI))) || (defined(NEXT) || defined(MACOS) || defined(MKLINUX) || defined(SUN) || defined(WINDOZE) || (!defined(PI)))
  #define PI 3.141592653589793
#endif

#ifndef SEEK_SET
  #define SEEK_SET 0
#endif

#ifndef SEEK_CUR
  #define SEEK_CUR 1
#endif

#ifndef SEEK_END
  #define SEEK_END 2
#endif

#ifndef MACOS
#ifndef CLM_SIGFNC_DEFINED
#define CLM_SIGFNC_DEFINED
  #ifndef RETSIGTYPE 
    #define RETSIGTYPE void
  #endif
  typedef RETSIGTYPE sigfnc(int);
#endif
#endif

#define SNDLIB_DAC_CHANNEL 252525
#define SNDLIB_DAC_REVERB 252520
#define SNDLIB_SNDFIX 32768.0
#define SNDLIB_SNDFLT 0.000030517578

#define unsupported_sound_file -1
#define NeXT_sound_file 0
#define AIFF_sound_file 1
#define RIFF_sound_file 2
#define BICSF_sound_file 3
#define NIST_sound_file 4
#define INRS_sound_file 5
#define ESPS_sound_file 6
#define SVX_sound_file 7
#define VOC_sound_file 8
#define SNDT_sound_file 9
#define raw_sound_file 10
#define SMP_sound_file 11
#define SD2_sound_file 12
#define AVR_sound_file 13
#define IRCAM_sound_file 14
#define SD1_sound_file 15
#define SPPACK_sound_file 16
#define MUS10_sound_file 17
#define HCOM_sound_file 18
#define PSION_sound_file 19
#define MAUD_sound_file 20
#define IEEE_sound_file 21
#define DeskMate_sound_file 22
#define DeskMate_2500_sound_file 23
#define Matlab_sound_file 24
#define ADC_sound_file 25
#define SoundEdit_sound_file 26
#define SoundEdit_16_sound_file 27
#define DVSM_sound_file 28
#define MIDI_file 29
#define Esignal_file 30
#define soundfont_sound_file 31
#define gravis_sound_file 32
#define comdisco_sound_file 33
#define goldwave_sound_file 34
#define srfs_sound_file 35
#define MIDI_sample_dump 36
#define DiamondWare_sound_file 37
#define RealAudio_sound_file 38
#define ADF_sound_file 39
#define SBStudioII_sound_file 40
#define Delusion_sound_file 41
#define Farandole_sound_file 42
#define Sample_dump_sound_file 43
#define Ultratracker_sound_file 44
#define Yamaha_SY85_sound_file 45
#define Yamaha_TX16_sound_file 46
#define digiplayer_sound_file 47
#define Covox_sound_file 48
#define SPL_sound_file 49
#define AVI_sound_file 50
#define OMF_sound_file 51
#define Quicktime_sound_file 52
#define asf_sound_file 53
#define Yamaha_SY99_sound_file 54
#define Kurzweil_2000_sound_file 55
#define old_style_AIFF_sound_file 56


#define snd_unsupported -1
#define snd_no_snd 0
#define snd_16_linear 1
#define snd_8_mulaw 2
#define snd_8_linear 3
#define snd_32_float 4
#define snd_32_linear 5
#define snd_8_alaw 6
#define snd_8_unsigned 7
#define snd_24_linear 8
#define snd_64_double 9
#define snd_16_linear_little_endian 10
#define snd_32_linear_little_endian 11
#define snd_32_float_little_endian 12
#define snd_64_double_little_endian 13
#define snd_16_unsigned 14
#define snd_16_unsigned_little_endian 15
#define snd_24_linear_little_endian 16
#define snd_32_vax_float 17
#define snd_12_linear 18
#define snd_12_linear_little_endian 19
#define snd_12_unsigned 20
#define snd_12_unsigned_little_endian 21
/* 64-bit ints apparently can occur in ESPS files */

#define NIST_shortpack 2
#define AIFF_IMA_ADPCM 99

#define DEFAULT_DEVICE 0
#define READ_WRITE_DEVICE 1
#define ADAT_IN_DEVICE 2
#define AES_IN_DEVICE 3
#define LINE_OUT_DEVICE 4
#define LINE_IN_DEVICE 5
#define MICROPHONE_DEVICE 6
#define SPEAKERS_DEVICE 7
#define DIGITAL_IN_DEVICE 8
#define DIGITAL_OUT_DEVICE 9
#define DAC_OUT_DEVICE 10
#define ADAT_OUT_DEVICE 11
#define AES_OUT_DEVICE 12
#define DAC_FILTER_DEVICE 13
#define MIXER_DEVICE 14
#define LINE1_DEVICE 15
#define LINE2_DEVICE 16
#define LINE3_DEVICE 17
#define AUX_INPUT_DEVICE 18
#define CD_IN_DEVICE 19
#define AUX_OUTPUT_DEVICE 20
#define SPDIF_IN_DEVICE 21
#define SPDIF_OUT_DEVICE 22

#define AUDIO_SYSTEM(n) ((n)<<16)
#define SNDLIB_SYSTEM(n) (((n)>>16)&0xffff)
#define SNDLIB_DEVICE(n) ((n)&0xffff)

#ifndef WINDOZE
  #define NO_ERROR 0
#else
  #define NO_ERROR 0L
#endif
#define CHANNELS_NOT_AVAILABLE 1
#define SRATE_NOT_AVAILABLE 2
#define FORMAT_NOT_AVAILABLE 3
#define NO_INPUT_AVAILABLE 4
#define NO_OUTPUT_AVAILABLE 5
#define INPUT_BUSY 6
#define OUTPUT_BUSY 7
#define CONFIGURATION_NOT_AVAILABLE 8
#define INPUT_CLOSED 9
#define OUTPUT_CLOSED 10
#define IO_INTERRUPTED 11
#define NO_LINES_AVAILABLE 12
#define WRITE_ERROR 13
#define SIZE_NOT_AVAILABLE 14
#define DEVICE_NOT_AVAILABLE 15
#define CANT_CLOSE 16
#define CANT_OPEN 17
#define READ_ERROR 18
#define AMP_NOT_AVAILABLE 19
#define AUDIO_NO_OP 20
#define CANT_WRITE 21
#define CANT_READ 22
#define NO_READ_PERMISSION 23

#define AMP_FIELD 0
#define SRATE_FIELD 1
#define CHANNEL_FIELD 2
#define FORMAT_FIELD 3
#define DEVICE_FIELD 4
#define IMIX_FIELD 5
#define IGAIN_FIELD 6
#define RECLEV_FIELD 7
#define PCM_FIELD 8
#define PCM2_FIELD 9
#define OGAIN_FIELD 10
#define LINE_FIELD 11
#define MIC_FIELD 12
#define LINE1_FIELD 13
#define LINE2_FIELD 14
#define LINE3_FIELD 15
#define SYNTH_FIELD 16
#define BASS_FIELD 17
#define TREBLE_FIELD 18
#define CD_FIELD 19


/* realloc is enough of a mess that I'll handle each case individually */

#ifdef MACOS
  /* C's calloc/free are incompatible with Mac's SndDisposeChannel (which we can't avoid using) */
  #define CALLOC(a,b)  NewPtrClear((a) * (b))
  #define MALLOC(a,b)  NewPtr((a) * (b))
  #define FREE(a)      DisposePtr((Ptr)(a))
#else
  #define CALLOC(a,b)  calloc(a,b)
  #define MALLOC(a,b)  malloc(a,b)
  #define FREE(a)      free(a)
  #define REALLOC(a,b) realloc(a,b)
#endif 


__BEGIN_DECLS
int sound_samples __P((char *arg));
int sound_datum_size __P((char *arg));
int sound_data_location __P((char *arg));
int sound_chans __P((char *arg));
int sound_srate __P((char *arg));
int sound_header_type __P((char *arg));
int sound_data_format __P((char *arg));
int sound_original_format __P((char *arg));
int sound_comment_start __P((char *arg));
int sound_comment_end __P((char *arg));
int sound_length __P((char *arg));
int sound_fact_samples __P((char *arg));
int sound_distributed __P((char *arg));
int sound_write_date __P((char *arg));
int sound_type_specifier __P((char *arg));
int sound_align __P((char *arg));
int sound_bits_per_sample __P((char *arg));
char *sound_type_name __P((int type));
char *sound_format_name __P((int format));
char *sound_comment __P((char *name));
int bytes_per_sample __P((int format));
void initialize_sndlib __P((void));
int override_sound_header __P((char *arg, int srate, int chans, int format, int type, int location, int size));

int open_sound_input __P((char *arg));
int open_sound_output __P((char *arg, int srate, int chans, int data_format, int header_type, char *comment));
int close_sound_input __P((int fd));
int close_sound_output __P((int fd, int bytes_of_data));
int read_sound __P((int fd, int beg, int end, int chans, int **bufs));
int write_sound __P((int tfd, int beg, int end, int chans, int **bufs));
int seek_sound __P((int tfd, long offset, int origin));

void describe_audio_state __P((void));
char *report_audio_state __P((void));
int open_audio_output __P((int dev, int srate, int chans, int format, int size));
int open_audio_input __P((int dev, int srate, int chans, int format, int size));
int write_audio __P((int line, char *buf, int bytes));
int close_audio __P((int line));
int read_audio __P((int line, char *buf, int bytes));
int read_audio_state __P((int dev, int field, int chan, float *val));
int write_audio_state __P((int dev, int field, int chan, float *val));
void save_audio_state __P((void));
void restore_audio_state __P((void));
int audio_error __P((void));
int initialize_audio __P((void));
char *audio_error_name __P((int err));
void set_audio_error __P((int err));
int audio_systems __P((void));
char *audio_system_name __P((int system));
char *audio_moniker __P((void));
#if HAVE_OSS
  void setup_dsps __P((int cards, int *dsps, int *mixers));
  void set_oss_buffers __P((int num,int size));
#endif
void write_mixer_state __P((char *file));
void read_mixer_state __P((char *file));

void clm_printf __P((char *str));

void normarray __P((int size, float *arr));
int get_shift_24_choice __P((void));
void set_shift_24_choice __P((int choice));
long excl_timedabsmaxarr __P((int beg, int end, int *maxA, int *arr));
long excl_clm_seek __P((int tfd, int *offset, int origin));
void set_clm_datum_type __P((int tfd, int type));

#if LONG_INT_P
  int *delist_ptr __P((int arr));
  int list_ptr __P((int *arr));
  int setarray __P((int arr_1, int i, int val));
  int getarray __P((int arr_1, int i));
  int incarray __P((int arr_1, int i, int val));
  int makearray __P((int len));
  void freearray __P((int ip_1));
  void cleararray1 __P((int beg, int end, int arr_1));
  void arrblt __P((int beg, int end, int newbeg, int arr_1));
  int absmaxarr __P((int beg, int end, int arr_1));
#else
  int setarray __P((int *arr, int i, int val));
  int getarray __P((int *arr, int i));
  int incarray __P((int *arr, int i, int val));
  int *makearray __P((int len));
  void freearray __P((int *ip));
  void cleararray1 __P((int beg, int end, int *arr));
  void arrblt __P((int beg, int end, int newbeg, int *arr));
  int absmaxarr __P((int beg, int end, int *arr));
#endif
void reset_io_c __P((void));
void reset_headers_c __P((void));
void reset_audio_c __P((void));
#ifndef MACOS
  sigfnc *clm_signal __P((int signo, sigfnc *fnc));
#endif
void set_rt_audio_p __P((int rt));

void open_clm_file_descriptors __P((int tfd, int df, int ds, int dl));
void close_clm_file_descriptors __P((int tfd));
void cleanup_clm_file_descriptors __P((void));
int clm_open_read __P((char *arg));
int clm_open_write __P((char *arg));
int clm_create __P((char *arg));
int clm_reopen_write __P((char *arg));
void clm_close __P((int fd));
long clm_seek __P((int tfd, long offset, int origin));
void clm_read __P((int fd, int beg, int end, int chans, int **bufs));
void clm_read_chans __P((int fd, int beg, int end, int chans, int **bufs, int *cm));
void clm_write_zeros __P((int tfd, int num));
void clm_write __P((int tfd, int beg, int end, int chans, int **bufs));

void float_sound __P((char *charbuf, int samps, int charbuf_format, float *buffer));
int c_snd_header_data_size __P((void));
int c_snd_header_data_location __P((void));
int c_snd_header_chans __P((void));
int c_snd_header_srate __P((void));
int c_snd_header_type __P((void));
int c_snd_header_format __P((void));
int c_snd_header_distributed __P((void));
int c_snd_header_comment_start __P((void));
int c_snd_header_comment_end __P((void));
int c_snd_header_type_specifier __P((void));
int c_snd_header_bits_per_sample __P((void));
int c_snd_header_fact_samples __P((void));
int c_snd_header_block_align __P((void));
int c_snd_header_loop_mode __P((int which));
int c_snd_header_loop_start __P((int which));
int c_snd_header_loop_end __P((int which));
int c_snd_header_mark_position __P((int id));
int c_snd_header_base_note __P((void));
int c_snd_header_base_detune __P((void));
int c_true_file_length __P((void));
int c_snd_header_original_format __P((void));
int c_snd_datum_size __P((int format));
int c_snd_header_datum_size __P((void));
int c_snd_bytes __P((int format, int size));
int c_snd_samples __P((int format, int size));
void write_next_header __P((int chan, int srate, int chans, int loc, int siz, int format, char *comment, int len));
void c_read_header_with_fd __P((int chan));
int c_read_header __P((char *name));
int c_write_header __P((char *name, int type, int srate, int chans, int loc, int size, int format, char *comment, int len));
int c_write_header_with_fd __P((int chan, int type, int in_srate, int in_chans, int loc, int size, int format, char *comment, int len));
void set_aifc_header __P((int val));
int c_update_header_with_fd __P((int chan, int type, int siz));
int c_update_header __P((char *name, int type, int size, int srate, int format, int chans, int loc));
int c_snd_header_aux_comment_start __P((int n));
int c_snd_header_aux_comment_end __P((int n));
int match_four_chars __P((unsigned char *head, const unsigned char *match));
int c_update_header_comment __P((char *name, int loc, char *comment, int len, int typ));
void create_header_buffer __P((void));
void create_descriptors __P((void));
int clm_read_any __P((int tfd, int beg, int chans, int nints, int **bufs, int *cm));
void c_set_snd_header __P((int in_srate, int in_chans, int in_format));
int unshort_sound __P((short *in_buf, int samps, int new_format, char *out_buf));

void set_big_endian_int __P((unsigned char *j, int x));
int get_big_endian_int __P((unsigned char *inp));
void set_little_endian_int __P((unsigned char *j, int x));
int get_little_endian_int __P((unsigned char *inp));
int get_uninterpreted_int __P((unsigned char *inp));
void set_big_endian_float __P((unsigned char *j, float x));
float get_big_endian_float __P((unsigned char *inp));
void set_little_endian_float __P((unsigned char *j, float x));
float get_little_endian_float __P((unsigned char *inp));
void set_big_endian_short __P((unsigned char *j, short x));
short get_big_endian_short __P((unsigned char *inp));
void set_little_endian_short __P((unsigned char *j, short x));
short get_little_endian_short __P((unsigned char *inp));
void set_big_endian_unsigned_short __P((unsigned char *j, unsigned short x));
unsigned short get_big_endian_unsigned_short __P((unsigned char *inp));
void set_little_endian_unsigned_short __P((unsigned char *j, unsigned short x));
unsigned short get_little_endian_unsigned_short __P((unsigned char *inp));
double get_little_endian_double __P((unsigned char *inp));
double get_big_endian_double __P((unsigned char *inp));
void set_big_endian_double __P((unsigned char *j, double x));
void set_little_endian_double __P((unsigned char *j, double x));
unsigned int get_big_endian_unsigned_int __P((unsigned char *inp));
unsigned int get_little_endian_unsigned_int __P((unsigned char *inp));

#ifdef CLM
int clm_read_floats __P((int fd,int n,float *arr));
int clm_read_swapped_floats __P((int fd,int n,float *arr));
int clm_read_ints __P((int fd,int n,int *arr));
int clm_read_swapped_ints __P((int fd,int n,int *arr));
int clm_write_floats __P((int fd,int n,float *arr));
void clm_seek_floats __P((int fd,int n));
void clm_seek_bytes __P((int fd,int n));
int clm_read_bytes __P((int fd,int n,char *arr));
int clm_write_bytes __P((int fd,int n,char *arr));
int excl_c_update_header __P((char *name, int type, int *siz, int srate, int format, int chans, int loc));
int excl_c_write_header __P((char *name, int type, int srate, int chans, int loc, int *siz, int format, char *comment, int len));
int net_mix __P((int fd, int loc, char *buf1, char *buf2, int bytes));

#ifdef MCL_PPC
  void clm_break __P((void));
  void clm_error __P((void));
  void clm_funcall __P((char *str));
  void set_lisp_callbacks __P((void (*lp)(char *),void (*bp)(void),void (*ep)(void),void (*fp)(char *)));
#endif

#endif

#ifdef SUN
void sun_outputs __P((int speakers, int headphones, int line_out));
#endif

#if (defined(HAVE_CONFIG_H)) && (!defined(HAVE_STRERROR))
  char *strerror __P((int errnum));
#endif

__END_DECLS

#endif
