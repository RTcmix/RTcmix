#include <endian.h>      /* so that sndlib.h will get host byte-order right */
#include "../sndlib/sndlib.h"
#include "../H/sfheader.h"       /* for SFHEADER */
#include "../rtstuff/rtdefs.h"   /* just for MAXCHANS */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* used to handle encoding peak stats in sound file comment */

/* all 3 constants are meant to include terminating NULL */
#define MAX_COMMENT_CHARS       512        /* not including peak stat text */
#define MAX_PEAK_CHARS          256
#define DEFAULT_COMMENT_LENGTH  (MAX_COMMENT_CHARS + MAX_PEAK_CHARS)

typedef struct {
   int     offset;          /* of peak info in comment; -1 if no peak info */
   float   peak[MAXCHANS];
   long    peakloc[MAXCHANS];
   long    timetag;         /* "calendar time" of last peak stats file update */
   char    comment[MAX_COMMENT_CHARS];
} SFComment;


/* functions in sys/sndlibsupport.c */

/* for creating, opening and closing files for sndlib I/O */
int sndlib_create(char *, int, int, int, int);
int sndlib_open_read(char *);
int sndlib_open_write(char *);
int sndlib_close(int, int, int, int, int);

/* for reading and writing headers */
int sndlib_read_header(int);
int sndlib_write_header(int, int, int, int, int, int, char *, int *);
int sndlib_set_header_data_size(int, int, int);
char *sndlib_print_current_header_stats(int, SFComment *, int);

/* for reading and writing header comments with encoded peak stats */
int sndlib_get_current_header_comment(int, SFComment *);
int sndlib_get_header_comment(int, SFComment *);
int sndlib_put_current_header_comment(int, float [], long [], char *);
int sndlib_put_header_comment(int, float [], long [], char *);

/* helper functions */
int sndlib_current_header_comment_alloc_good(char *);
void sndlib_current_header_match_aiff_flavor(void);
int sndlib_current_header_is_aifc(void);
int **sndlib_allocate_buffers(int, int);
void sndlib_free_buffers(int **, int);
int sndlib_findpeak(int, int, int, int, int, int, long, long,
                                                          float [], long []);

/* for cmix legacy support */
int sndlib_rheader(int, SFHEADER *);
int sndlib_wheader(int, SFHEADER *);
int getheadersize(SFHEADER *);
int putlength(char *, int, SFHEADER *);
char *getsfcode(SFHEADER *, int);
int putsfcode(SFHEADER *, char *, SFCODE *);
int getsfmaxamp(SFHEADER *, SFMAXAMP *);


/* some handy macros */

#define IS_FLOAT_FORMAT(format) (                        \
              (format) == snd_32_float                   \
           || (format) == snd_32_float_little_endian     )

#define NOT_A_SOUND_FILE(header_type) (                  \
              (header_type) == unsupported_sound_file    \
           || (header_type) == raw_sound_file            )

/* The header types that sndlib can write, as of sndlib-5.5. */
#define WRITEABLE_HEADER_TYPE(type) (                    \
               (type) == AIFF_sound_file                 \
           ||  (type) == NeXT_sound_file                 \
           ||  (type) == RIFF_sound_file                 \
           ||  (type) == IRCAM_sound_file                )

#define INVALID_DATA_FORMAT(format) ((format) < 1)

/* The data formats that disk-based cmix code can read and write. */
#define SUPPORTED_DATA_FORMAT(format) (                  \
               (format) == snd_16_linear                 \
           ||  (format) == snd_16_linear_little_endian   \
           ||  (format) == snd_32_float                  \
           ||  (format) == snd_32_float_little_endian    )

/* Usually endianness is handled for us by sndlib, but there are some
   cases where it's helpful to know (e.g., sfcreate). These are the
   more common formats sndlib supports -- not all of them.
*/
#define IS_BIG_ENDIAN_FORMAT(format) (                   \
              (format) == snd_16_linear                  \
           || (format) == snd_32_float                   \
           || (format) == snd_24_linear                  \
           || (format) == snd_32_linear                  \
           || (format) == snd_16_unsigned                )

#define IS_LITTLE_ENDIAN_FORMAT(format) (                \
              (format) == snd_16_linear_little_endian    \
           || (format) == snd_32_float_little_endian     \
           || (format) == snd_24_linear_little_endian    \
           || (format) == snd_32_linear_little_endian    \
           || (format) == snd_16_unsigned_little_endian  )

#define FORMATS_SAME_BYTE_ORDER(format1, format2) (      \
             ( IS_BIG_ENDIAN_FORMAT(format1)             \
            && IS_BIG_ENDIAN_FORMAT(format2) )           \
         ||  ( IS_LITTLE_ENDIAN_FORMAT(format1)          \
            && IS_LITTLE_ENDIAN_FORMAT(format2) )        )

#define SFCOMMENT_PEAKSTATS_VALID(sfcptr)  ((sfcptr)->offset != -1)


/* These macros nabbed from sndlib/io.c, since they aren't exposed to us,
   and they're useful for doing our own float i/o.
*/

#ifdef SNDLIB_LITTLE_ENDIAN
  #define m_big_endian_float(n)                   (get_big_endian_float(n))
  #define m_little_endian_float(n)                (*((float *)n))
  #define m_set_big_endian_float(n,x)             set_big_endian_float(n,x)
  #define m_set_little_endian_float(n,x)          (*((float *)n)) = x
#else
  #ifndef SUN
    #define m_big_endian_float(n)                   (*((float *)n))
    #define m_set_big_endian_float(n,x)             (*((float *)n)) = x
  #else
    #define m_big_endian_float(n)                   (get_big_endian_float(n))
    #define m_set_big_endian_float(n,x)             set_big_endian_float(n,x)
  #endif
  #define m_little_endian_float(n)                  (get_little_endian_float(n))
  #define m_set_little_endian_float(n,x)            set_little_endian_float(n,x)
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

