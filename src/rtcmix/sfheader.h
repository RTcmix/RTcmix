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


# define SF_BUFSIZE	(16*1024)
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

/* NeXT macros */

# define sfchans(x) (x)->sfinfo.sf_chans
# define sfmagic(x) (x)->sfinfo.sf_magic

# define sfsrate(x) (x)->sfinfo.sf_srate
# define sfclass(x) (x)->sfinfo.sf_packmode
# define sfbsize(x) ((x)->st_size - sizeof(SFHEADER))
# define sfcodes(x) (x)->sfinfo.sf_codes

# define islink(x)  ((x)->sfinfo.sf_magic == SF_LINK)

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

#define readopensf(name,fd,sfh,sfst,prog,result) \
if ((fd = open(name,0))  < 0) {  \
        fprintf(stderr,"%s: cannot access file %s\n",prog,name); \
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
} \
else if (stat(name,&sfst)){ \
	fprintf(stderr,"%s: cannot get status on %s\n",prog,name); \
	result = -1;  \
} \
else result = 0;

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
