#ifndef _SFHEADER_H_ 
#define _SFHEADER_H_ 1        /* To avoid recursion in certain includes */

# define SIZEOF_BSD_HEADER 1024
/* and for backward compatibility... */
# define SIZEOF_HEADER 1024

#ifdef sgi
#include <audiofile.h>
#define SGI_MAGIC 'AIFF' /* SGI compiler allows 4-char int constants */
#endif

#if defined(NeXT) || defined(NEXT)
#define SIZEOF_NeXT_HEADER 28
#else
#define SIZEOF_NeXT_HEADER SIZEOF_BSD_HEADER
#endif /* NeXT, etc. */

/* Next SND header */
# define SND_MAGIC ((int)0x2e736e64)
/* same value, byte-swapped */
# define SND_SWAPMAGIC ((int)0x646e732e)

/* used by old BSD/IRCAM style headers */
# define SF_MAGIC 0x0001a364
/* same value, byte-swapped */
# define SF_SWAPMAGIC 0x64a30100

# define SF_LINK 107414

/* Some IRCAM sound formation information */
# define SF_SHORT sizeof(short)
# define SF_FLOAT sizeof(float)

/* Some NEXT sound format information */
# define SND_FORMAT_MULAW_8    (1)
# define SND_FORMAT_LINEAR_8   (2)
# define SND_FORMAT_LINEAR_16  (3)
# define SND_FORMAT_LINEAR_24  (4)
# define SND_FORMAT_LINEAR_32  (5)
# define SND_FORMAT_FLOAT      (6)
# define SND_FORMAT_DOUBLE     (7)


# define SF_BUFSIZE	(16*1024)  /* JGG: should double this */
# define SF_MAXCHAN	4
# define MAXCOMM 512
# define MINCOMM 256

/* Codes for sfcode */
# define SF_END 0
# define SF_MAXAMP 1
# define SF_COMMENT 2
# define SF_LINKCODE 3

typedef struct sfcode {
	short	code;
	short	bsize;
} SFCODE;

typedef struct sfmaxamp {
	float	value[SF_MAXCHAN];
	long	samploc[SF_MAXCHAN];
	long	timetag;
} SFMAXAMP;

typedef struct sfcomment {
	char 	comment[MAXCOMM];
} SFCOMMENT;

typedef struct sflink {
	char 	reality[50];
	int 	startsamp;
	int	endsamp;
} SFLINK;


#ifdef USE_SNDLIB

/* The strategy for adding sndlib support to disk-based cmix is to
   hook it in at the lowest level possible, so that old code can go
   about its business without changes, for the most part. The changes
   to SFHEADER let us keep a lot of the macros used by instruments
   and other code.
*/

/* NOTE: We don't read and write this to files directly.
   The sfinfo struct is unfortunate; it's here for backwards compatibility.
*/
typedef union sfheader {
   struct {
      short    header_type;    /* sndlib constant for "aif", "wav", etc */
      short    data_format;    /* sndlib constant (short, float, byte-order) */
      int      data_location;  /* offset in bytes to sound data */
      long     data_size;      /* size in bytes of sound data */
      float    sf_srate;
      int      sf_chans;
      int      sf_packmode;    /* aka: bytes per sample */
      SFMAXAMP sf_maxamp;
      char     sf_comment[MAXCOMM];
   } sfinfo;
} SFHEADER;

#else /* !USE_SNDLIB */

/* This is the old Next-style data */
/* Nothing gets written here though */

typedef struct {
    int magic;          /* must be equal to SND_MAGIC */
    int dataLocation;   /* Offset or pointer to the raw data */
    int dataSize;       /* Number of bytes of data in the raw data */
    int dataFormat;     /* The data format code */
    int samplingRate;   /* The sampling rate */
    int channelCount;   /* The number of channels */
    char info[4];       /* Textual information relating to the sound. */
} SNDSoundStruct;

/* Had to include the old Next header stuff */
typedef union sfheader {
  struct {
    SNDSoundStruct NeXTheader;  /* Blech! */
    int	  sf_magic;
    float	  sf_srate;
    int	  sf_chans;
    int	  sf_packmode;
    char	  sf_codes;
  } sfinfo;
  char	filler[SIZEOF_BSD_HEADER];
} SFHEADER;

#endif /* !USE_SNDLIB */

/* NeXT macros */

# define sfchans(x) (x)->sfinfo.sf_chans
#ifndef USE_SNDLIB
# define sfmagic(x) (x)->sfinfo.sf_magic
#endif
# define sfsrate(x) (x)->sfinfo.sf_srate
# define sfclass(x) (x)->sfinfo.sf_packmode

#ifdef USE_SNDLIB
/* Macros to access sndlib-specific fields */
# define sfheadertype(x) ((x)->sfinfo.header_type)
# define sfdataformat(x) ((x)->sfinfo.data_format)
# define sfdatalocation(x) ((x)->sfinfo.data_location)
# define sfmaxampstruct(x) ((x)->sfinfo.sf_maxamp)
# define sfcommentstr(x) ((x)->sfinfo.sf_comment)

/* Not enough info passed in for sfbsize, so can't use it (assumes a fixed
   header size). Caller should use the following macro instead, when using
   sndlib. Note that <x> is ptr to SFHEADER, not a stat buffer.
*/
# define sfdatasize(x) (((x))->sfinfo.data_size)

/* Don't need sfcodes(). Can't use islink(). */
#else /* !USE_SNDLIB */
# define sfbsize(x) ((x)->st_size - sizeof(SFHEADER))
# define sfcodes(x) (x)->sfinfo.sf_codes
# define islink(x)  ((x)->sfinfo.sf_magic == SF_LINK)
#endif /* !USE_SNDLIB */

# define sfmaxamp(mptr,chan) (mptr)->value[chan]
# define sfmaxamploc(mptr,chan) (mptr)->samploc[chan]
# define sfmaxamptime(x) (x)->timetag
# define ismaxampgood(x,s) (sfmaxamptime(x) + 2  >= (s)->st_mtime)

# define sfcomm(x,n) (x)->comment[n]

# define realname(x) (x)->reality
# define startsmp(x) (x)->startsamp
# define endsmp(x) (x)->endsamp
# define sfoffset(x,h) ((x)->startsamp * sfchans(h) * sfclass(h))
# define sfendset(x,h) ((x)->endsamp * sfchans(h) * sfclass(h))

#ifdef USE_SNDLIB

/* sflseek(): must fix caller so as not to assume header size when
   using sndlib (see lib/getsample.c for example).
*/

/* These functions in sys/sndlibsupport.c
   <sfh> is pointer to an SFHEADER.
   Code that calls these should include H/sndlibsupport.h.
*/
# define wheader(fd, sfh) sndlib_wheader((fd), (SFHEADER *)(sfh))
# define rheader(fd, sfh) sndlib_rheader((fd), (SFHEADER *)(sfh))

/* Don't need the file magic macros. */

/* prototypes to suppress compiler warnings */
int check_byte_order(SFHEADER *, char *, char *);
void printsf(SFHEADER *);

#else /* !USE_SNDLIB */

# define sflseek(x,y,z) lseek(x,z != 0 ? y : (y) + sizeof(SFHEADER),z)

#if defined(NeXT) || defined(NEXT)
/* special routines for reading and writing various headers */
# define wheader(x,y) writeHeader(x,y)
# define rheader(x,y) readHeader(x,y)
# define nsflseek(x,y,z) lseek(x,z != 0 ? y : ((y) + getheadersize(x)),z)
# define nsfmagic(x) (x)->sfinfo.NeXTheader.magic
# define ismagic(x) (1)
#else

# define rheader(x,y) read(x,y,sizeof(SFHEADER)) != sizeof(SFHEADER)
# define wheader(x,y) write(x,y,sizeof(SFHEADER)) != sizeof(SFHEADER)
# define ismagic(x) ((x)->sfinfo.sf_magic == SF_MAGIC)
# define is_swapmagic(x) ((x)->sfinfo.sf_magic == SF_SWAPMAGIC)
# define is_nsfmagic(x) ((x)->sfinfo.NeXTheader.magic == SND_MAGIC)
# define is_nsfswapmagic(x) ((x)->sfinfo.NeXTheader.magic == SND_SWAPMAGIC)

/* Some handy macros to help us with the older Next-style soundfile headers */
# define NSchans(x) (x)->sfinfo.NeXTheader.channelCount
# define NSmagic(x) (x)->sfinfo.NeXTheader.magic
# define NSsrate(x) (x)->sfinfo.NeXTheader.samplingRate
# define NSclass(x) (x)->sfinfo.NeXTheader.dataFormat
# define NSdloc(x)  (x)->sfinfo.NeXTheader.dataLocation
# define NSdsize(x)  (x)->sfinfo.NeXTheader.dataSize
#endif

#endif /* !USE_SNDLIB */

#define readopensf(name,fd,sfh,sfst,prog,result) \
if ((fd = open(name,O_RDONLY)) < 0) {  \
        fprintf(stderr,"%s: cannot access file %s\n",prog,name); \
	result = -1;  \
} \
else if (rheader(fd,&sfh)){ \
	fprintf(stderr,"%s: cannot read header from %s\n",prog,name); \
	result = -1;  \
} \
else if (check_byte_order(&sfh,prog,name)) { \
	fprintf(stderr,"%s: file is unrecognizable %s\n",prog,name); \
	result = -1;  \
} \
else if (stat(name,&sfst)){ \
	fprintf(stderr,"%s: cannot get status on %s\n",prog,name); \
	result = -1;  \
} \
else result = 0;

#define rwopensf(name,fd,sfh,sfst,prog,result,code) \
if ((fd = open(name, code))  < 0) {  \
        fprintf(stderr,"%s: cannot access file %s\n",prog,name); \
	result = -1;  \
} \
else if (rheader(fd,&sfh)){ \
	fprintf(stderr,"%s: cannot read header from %s\n",prog,name); \
	result = -1;  \
} \
else if (check_byte_order(&sfh,prog,name)) { \
	fprintf(stderr,"%s: file is unrecognizable %s\n",prog,name); \
	result = -1;  \
} \
else if (stat(name,&sfst)){ \
	fprintf(stderr,"%s: cannot get status on %s\n",prog,name); \
	result = -1;  \
} \
else result = 0;

#ifndef USE_SNDLIB

#define newrwopensf(name,fd,sfh,sfst,prog,result,code) \
if ((fd = open(name, code))  < 0) {  \
	result = -1;  \
} \
else if (rheader(fd,&sfh)){ \
	fprintf(stderr,"%s: cannot read header from %s\n",prog,name); \
	result = -1;  \
} \
else if (check_byte_order(&sfh,prog,name)) { \
	fprintf(stderr,"%s: file is unrecognizable %s\n",prog,name); \
} \
else if (stat(name,&sfst)){ \
	fprintf(stderr,"%s: cannot get status on %s\n",prog,name); \
	result = -1;  \
} \
else if (ismagic(&sfh)) { \
	fprintf(stdout, "%s is a BSD/IRCAM snd file\n",name); \
        result = 0; \
} \
else if (is_swapmagic(&sfh)) { \
	fprintf(stdout, "%s is a byteswapped BSD/IRCAM snd file\n",name); \
        result = 0; \
} \
else result = 0;

#endif /* !USE_SNDLIB */

#endif /* _SFHEADER_H_ */
