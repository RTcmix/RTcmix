#include "../rtstuff/rtdefs.h"   /* just for MAXCHANS */
#include "../sndlib/sndlib.h"

/* used to handle encoding peak stats in sound file comment */

#define MAX_COMMENT_CHARS       512        /* not including peak stat text */
#define MAX_PEAK_CHARS          512
#define DEFAULT_COMMENT_LENGTH  (MAX_COMMENT_CHARS + MAX_PEAK_CHARS)

typedef struct {
   int     offset;            /* of peak info in comment; -1 if no peak info */
   float   peak[MAXCHANS];
   long    peakloc[MAXCHANS];
   char    comment[MAX_COMMENT_CHARS];
} SFComment;


/* functions in sys/sndlibsupport.c */

int sndlib_get_header_comment(char *, SFComment *);
int sndlib_get_current_header_comment(char *, SFComment *);
int sndlib_set_header_comment(char *, float [], long [], char *);
int sndlib_close_file(int);


/* some handy macros */

#define IS_FLOAT_FORMAT(format) (                        \
              (format) == snd_32_float                   \
           || (format) == snd_32_float_little_endian     )

/* This is for the disk-based i/o code in sound.c */
#define SUPPORTED_DATA_FORMAT(format) (                  \
               (format) == snd_16_linear                 \
           ||  (format) == snd_16_linear_little_endian   \
           ||  (format) == snd_32_float                  \
           ||  (format) == snd_32_float_little_endian    )


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

