/* IO handlers */
/*
 * --------------------------------
 * void create_descriptors (void): initialize (allocate) various global arrays
 * void clm_read(int fd, int beg, int end, int chans, int **bufs)
 * void clm_write(int tfd, int beg, int end, int chans, int **bufs)
 * long clm_seek(int tfd, long offset, int origin)
 * int clm_open_read(char *arg) 
 * int clm_open_write(char *arg)
 * int clm_create(char *arg)
 * int clm_reopen_write(char *arg)
 * void clm_close(int fd)
 * void open_clm_file_descriptors (int tfd, int df, int ds, int dl)
 * void close_clm_file_descriptors(int tfd)
 * see sndplay.c for a short example
 * --------------------------------
 */

#if defined(HAVE_CONFIG_H)
  #include "config.h"
#endif

#include <math.h>
#include <stdio.h>
#if (!defined(HAVE_CONFIG_H)) || (defined(HAVE_FCNTL_H))
  #include <fcntl.h>
#endif
#include <signal.h>
#if (!defined(HAVE_CONFIG_H)) || (defined(HAVE_LIMITS_H))
  #include <limits.h>
#endif
#include <errno.h>
#include <stdlib.h>

#if (defined(NEXT) || (defined(HAVE_LIBC_H) && (!defined(HAVE_UNISTD_H))))
  #include <libc.h>
#else
  #if (!(defined(_MSC_VER))) && (!(defined(MPW_C)))
    #include <unistd.h>
  #endif
#endif

#ifndef MACOS
  #include <string.h>
#endif

#if (!defined(NEXT)) && (!defined(inline))
  #define inline
#endif

#if (defined(SIZEOF_INT) && (SIZEOF_INT != 4)) || (defined(INT_MAX) && (INT_MAX != 2147483647))
  #error sndlib C code assumes 32-bit ints
#endif

#if (defined(SIZEOF_LONG) && (SIZEOF_LONG < 4)) || (defined(LONG_MAX) && (LONG_MAX < 2147483647))
  #error sndlib C code assumes longs are at least 32 bits
#endif

#if (defined(SIZEOF_SHORT) && (SIZEOF_SHORT != 2)) || (defined(SHRT_MAX) && (SHRT_MAX != 32767))
  #error sndlib C code assumes 16-bit shorts
#endif

#if (defined(SIZEOF_CHAR) && (SIZEOF_CHAR != 1)) || (defined(CHAR_BIT) && (CHAR_BIT != 8))
  #error sndlib C code assumes 8-bit chars
#endif

#include "sndlib.h"

#ifdef CLM
/* in MCL, fprintf causes the Mac to crash, so we go through clm_printf for all such calls */

#ifdef MCL_PPC
  void (*lisp_printf_callback)(char *);
  void (*lisp_break_callback)(void);
  void (*lisp_error_callback)(void);
  void (*lisp_funcall_callback)(char *);
  void set_lisp_callbacks(void (*lp)(char *),void (*bp)(void),void (*ep)(void),void (*fp)(char *))
  {
    lisp_printf_callback = lp;
    lisp_break_callback = bp;
    lisp_error_callback = ep;
    lisp_funcall_callback = fp;
  }
void clm_break(void) {(*lisp_break_callback)();}
void clm_error(void) {(*lisp_error_callback)();}
void clm_funcall(char *str) {(*lisp_funcall_callback)(str);}
#endif
#endif

void clm_printf(char *str)
{
#ifdef MCL_PPC 
  (*lisp_printf_callback)(str);
#else
  #ifdef ACL_LITE
    return;
  #else
    #if defined(WINDOZE) && defined(ACL4)
      aclprintf(str);
    #else
      fprintf(stdout,str); fflush(stdout);
    #endif
  #endif
#endif
}


/* data translations for big/little endian machines -- the m_* forms are macros where possible for speed */

void set_big_endian_int(unsigned char *j, int x)
{
  unsigned char *ox;
  ox=(unsigned char *)&x;
#ifdef SNDLIB_LITTLE_ENDIAN
  j[0]=ox[3]; j[1]=ox[2]; j[2]=ox[1]; j[3]=ox[0];
#else
  j[0]=ox[0]; j[1]=ox[1]; j[2]=ox[2]; j[3]=ox[3];
#endif
}

int get_big_endian_int (unsigned char *inp)
{
  int o;
  unsigned char *outp;
  outp=(unsigned char *)&o;
#ifdef SNDLIB_LITTLE_ENDIAN
  outp[0]=inp[3]; outp[1]=inp[2]; outp[2]=inp[1]; outp[3]=inp[0];
#else
  outp[0]=inp[0]; outp[1]=inp[1]; outp[2]=inp[2]; outp[3]=inp[3];
#endif
  return(o);
}

void set_little_endian_int(unsigned char *j, int x)
{
  unsigned char *ox;
  ox=(unsigned char *)&x;
#ifndef SNDLIB_LITTLE_ENDIAN
  j[0]=ox[3]; j[1]=ox[2]; j[2]=ox[1]; j[3]=ox[0];
#else
  j[0]=ox[0]; j[1]=ox[1]; j[2]=ox[2]; j[3]=ox[3];
#endif
}

int get_little_endian_int (unsigned char *inp)
{
  int o;
  unsigned char *outp;
  outp=(unsigned char *)&o;
#ifndef SNDLIB_LITTLE_ENDIAN
  outp[0]=inp[3]; outp[1]=inp[2]; outp[2]=inp[1]; outp[3]=inp[0];
#else
  outp[0]=inp[0]; outp[1]=inp[1]; outp[2]=inp[2]; outp[3]=inp[3];
#endif
  return(o);
}

int get_uninterpreted_int (unsigned char *inp)
{
  int o;
  unsigned char *outp;
  outp=(unsigned char *)&o;
  outp[0]=inp[0]; outp[1]=inp[1]; outp[2]=inp[2]; outp[3]=inp[3];
  return(o);
}

unsigned int get_big_endian_unsigned_int (unsigned char *inp)
{
  unsigned int o;
  unsigned char *outp;
  outp=(unsigned char *)&o;
#ifdef SNDLIB_LITTLE_ENDIAN
  outp[0]=inp[3]; outp[1]=inp[2]; outp[2]=inp[1]; outp[3]=inp[0];
#else
  outp[0]=inp[0]; outp[1]=inp[1]; outp[2]=inp[2]; outp[3]=inp[3];
#endif
  return(o);
}

unsigned int get_little_endian_unsigned_int (unsigned char *inp)
{
  unsigned int o;
  unsigned char *outp;
  outp=(unsigned char *)&o;
#ifndef SNDLIB_LITTLE_ENDIAN
  outp[0]=inp[3]; outp[1]=inp[2]; outp[2]=inp[1]; outp[3]=inp[0];
#else
  outp[0]=inp[0]; outp[1]=inp[1]; outp[2]=inp[2]; outp[3]=inp[3];
#endif
  return(o);
}


void set_big_endian_float(unsigned char *j, float x)
{
  unsigned char *ox;
  ox=(unsigned char *)&x;
#ifdef SNDLIB_LITTLE_ENDIAN
  j[0]=ox[3]; j[1]=ox[2]; j[2]=ox[1]; j[3]=ox[0];
#else
  j[0]=ox[0]; j[1]=ox[1]; j[2]=ox[2]; j[3]=ox[3];
#endif
}

float get_big_endian_float (unsigned char *inp)
{
  float o;
  unsigned char *outp;
  outp=(unsigned char *)&o;
#ifdef SNDLIB_LITTLE_ENDIAN
  outp[0]=inp[3]; outp[1]=inp[2]; outp[2]=inp[1]; outp[3]=inp[0];
#else
  outp[0]=inp[0]; outp[1]=inp[1]; outp[2]=inp[2]; outp[3]=inp[3];
#endif
  return(o);
}

void set_little_endian_float(unsigned char *j, float x)
{
  unsigned char *ox;
  ox=(unsigned char *)&x;
#ifndef SNDLIB_LITTLE_ENDIAN
  j[0]=ox[3]; j[1]=ox[2]; j[2]=ox[1]; j[3]=ox[0];
#else
  j[0]=ox[0]; j[1]=ox[1]; j[2]=ox[2]; j[3]=ox[3];
#endif
}

float get_little_endian_float (unsigned char *inp)
{
  float o;
  unsigned char *outp;
  outp=(unsigned char *)&o;
#ifndef SNDLIB_LITTLE_ENDIAN
  outp[0]=inp[3]; outp[1]=inp[2]; outp[2]=inp[1]; outp[3]=inp[0];
#else
  outp[0]=inp[0]; outp[1]=inp[1]; outp[2]=inp[2]; outp[3]=inp[3];
#endif
  return(o);
}

void set_big_endian_short(unsigned char *j, short x)
{
  unsigned char *ox;
  ox=(unsigned char *)&x;
#ifdef SNDLIB_LITTLE_ENDIAN
  j[0]=ox[1]; j[1]=ox[0];
#else
  j[0]=ox[0]; j[1]=ox[1];
#endif
}

short get_big_endian_short (unsigned char *inp)
{
  short o;
  unsigned char *outp;
  outp=(unsigned char *)&o;
#ifdef SNDLIB_LITTLE_ENDIAN
  outp[0]=inp[1]; outp[1]=inp[0];
#else
  outp[0]=inp[0]; outp[1]=inp[1];
#endif
  return(o);
}

void set_little_endian_short(unsigned char *j, short x)
{
  unsigned char *ox;
  ox=(unsigned char *)&x;
#ifndef SNDLIB_LITTLE_ENDIAN
  j[0]=ox[1]; j[1]=ox[0];
#else
  j[0]=ox[0]; j[1]=ox[1];
#endif
}

short get_little_endian_short (unsigned char *inp)
{
  short o;
  unsigned char *outp;
  outp=(unsigned char *)&o;
#ifndef SNDLIB_LITTLE_ENDIAN
  outp[0]=inp[1]; outp[1]=inp[0];
#else
  outp[0]=inp[0]; outp[1]=inp[1];
#endif
  return(o);
}

void set_big_endian_unsigned_short(unsigned char *j, unsigned short x)
{
  unsigned char *ox;
  ox=(unsigned char *)&x;
#ifdef SNDLIB_LITTLE_ENDIAN
  j[0]=ox[1]; j[1]=ox[0];
#else
  j[0]=ox[0]; j[1]=ox[1];
#endif
}

unsigned short get_big_endian_unsigned_short (unsigned char *inp)
{
  unsigned short o;
  unsigned char *outp;
  outp=(unsigned char *)&o;
#ifdef SNDLIB_LITTLE_ENDIAN
  outp[0]=inp[1]; outp[1]=inp[0];
#else
  outp[0]=inp[0]; outp[1]=inp[1];
#endif
  return(o);
}

void set_little_endian_unsigned_short(unsigned char *j, unsigned short x)
{
  unsigned char *ox;
  ox=(unsigned char *)&x;
#ifndef SNDLIB_LITTLE_ENDIAN
  j[0]=ox[1]; j[1]=ox[0];
#else
  j[0]=ox[0]; j[1]=ox[1];
#endif
}

unsigned short get_little_endian_unsigned_short (unsigned char *inp)
{
  unsigned short o;
  unsigned char *outp;
  outp=(unsigned char *)&o;
#ifndef SNDLIB_LITTLE_ENDIAN
  outp[0]=inp[1]; outp[1]=inp[0];
#else
  outp[0]=inp[0]; outp[1]=inp[1];
#endif
  return(o);
}

double get_little_endian_double (unsigned char *inp)
{
  double o;
#ifndef SNDLIB_LITTLE_ENDIAN
  int i;
#endif
  unsigned char *outp;
  outp=(unsigned char *)&o;
#ifndef SNDLIB_LITTLE_ENDIAN
  for (i=0;i<8;i++) outp[i]=inp[i];
#else
  outp[0]=inp[7]; outp[1]=inp[6]; outp[2]=inp[5]; outp[3]=inp[4]; outp[4]=inp[3]; outp[5]=inp[2]; outp[6]=inp[1]; outp[7]=inp[0];
#endif
  return(o);
}

double get_big_endian_double (unsigned char *inp)
{
  double o;
#ifdef SNDLIB_LITTLE_ENDIAN
  int i;
#endif
  unsigned char *outp;
  outp=(unsigned char *)&o;
#ifndef SNDLIB_LITTLE_ENDIAN
  outp[0]=inp[7]; outp[1]=inp[6]; outp[2]=inp[5]; outp[3]=inp[4]; outp[4]=inp[3]; outp[5]=inp[2]; outp[6]=inp[1]; outp[7]=inp[0];
#else
  for (i=0;i<8;i++) outp[i]=inp[i];
#endif
  return(o);
}

void set_big_endian_double(unsigned char *j, double x)
{
#ifdef SNDLIB_LITTLE_ENDIAN
  int i;
#endif
  unsigned char *ox;
  ox=(unsigned char *)&x;
#ifndef SNDLIB_LITTLE_ENDIAN
  j[0]=ox[7]; j[1]=ox[6]; j[2]=ox[5]; j[3]=ox[4]; j[4]=ox[3]; j[5]=ox[2]; j[6]=ox[1]; j[7]=ox[0];
#else
  for (i=0;i<8;i++) j[i]=ox[i];
#endif
}

void set_little_endian_double(unsigned char *j, double x)
{
#ifndef SNDLIB_LITTLE_ENDIAN
  int i;
#endif
  unsigned char *ox;
  ox=(unsigned char *)&x;
#ifndef SNDLIB_LITTLE_ENDIAN
  for (i=0;i<8;i++) j[i]=ox[i];
#else
  j[0]=ox[7]; j[1]=ox[6]; j[2]=ox[5]; j[3]=ox[4]; j[4]=ox[3]; j[5]=ox[2]; j[6]=ox[1]; j[7]=ox[0];
#endif
}

/* Vax float translation taken from Mosaic libdtm/vaxcvt.c */
static float from_vax_float(unsigned char *inp)
{
  unsigned char exp;
  unsigned char c0, c1, c2, c3;
  float o;
  unsigned char *outp;
  outp=(unsigned char *)&o;
  c0 = inp[0]; c1 = inp[1]; c2 = inp[2]; c3 = inp[3];
  exp = (c1 << 1) | (c0 >> 7);             /* extract exponent */
  if (!exp && !c1) return(0.0);            /* zero value */
  else if (exp>2) {                        /* normal value */
    outp[0] = c1 - 1;                      /* subtracts 2 from exponent */
    outp[1] = c0;                          /* copy mantissa, LSB of exponent */
    outp[2] = c3;
    outp[3] = c2;}
  else if (exp) {                          /* denormalized number */
    unsigned int shft;
    outp[0] = c1 & 0x80;                   /* keep sign, zero exponent */
    shft = 3 - exp;
    /* shift original mant by 1 or 2 to get denormalized mant */
    /* prefix mantissa with '1'b or '01'b as appropriate */
    outp[1] = ((c0 & 0x7f) >> shft) | (0x10 << exp);
    outp[2] = (c0 << (8-shft)) | (c3 >> shft);
    outp[3] = (c3 << (8-shft)) | (c2 >> shft);}
  else {                                   /* sign=1 -> infinity or NaN */
    outp[0] = 0xff;                        /* set exp to 255 */
    outp[1] = c0 | 0x80;                   /* LSB of exp = 1 */
    outp[2] = c3;
    outp[3] = c2;}
  return(o);
}


#ifdef SNDLIB_LITTLE_ENDIAN

  #define m_big_endian_short(n)                   (get_big_endian_short(n))
  #define m_big_endian_int(n)                     (get_big_endian_int(n))
  #define m_big_endian_float(n)                   (get_big_endian_float(n))
  #define m_big_endian_double(n)                  (get_big_endian_double(n))
  #define m_big_endian_unsigned_short(n)          (get_big_endian_unsigned_short(n))

  #define m_little_endian_short(n)                (*((short *)n))
  #define m_little_endian_int(n)                  (*((int *)n))
  #define m_little_endian_float(n)                (*((float *)n))
  #define m_little_endian_double(n)               (*((double *)n))
  #define m_little_endian_unsigned_short(n)       (*((unsigned short *)n))

  #define m_set_big_endian_short(n,x)             set_big_endian_short(n,x)
  #define m_set_big_endian_int(n,x)               set_big_endian_int(n,x)
  #define m_set_big_endian_float(n,x)             set_big_endian_float(n,x)
  #define m_set_big_endian_double(n,x)            set_big_endian_double(n,x)
  #define m_set_big_endian_unsigned_short(n,x)    set_big_endian_unsigned_short(n,x)

  #define m_set_little_endian_short(n,x)          (*((short *)n)) = x
  #define m_set_little_endian_int(n,x)            (*((int *)n)) = x
  #define m_set_little_endian_float(n,x)          (*((float *)n)) = x
  #define m_set_little_endian_double(n,x)         (*((double *)n)) = x
  #define m_set_little_endian_unsigned_short(n,x) (*((unsigned short *)n)) = x

#else

  #ifndef SUN
    #define m_big_endian_short(n)                   (*((short *)n))
    #define m_big_endian_int(n)                     (*((int *)n))
    #define m_big_endian_float(n)                   (*((float *)n))
    #define m_big_endian_double(n)                  (*((double *)n))
    #define m_big_endian_unsigned_short(n)          (*((unsigned short *)n))

    #define m_set_big_endian_short(n,x)             (*((short *)n)) = x
    #define m_set_big_endian_int(n,x)               (*((int *)n)) = x
    #define m_set_big_endian_float(n,x)             (*((float *)n)) = x
    #define m_set_big_endian_double(n,x)            (*((double *)n)) = x
    #define m_set_big_endian_unsigned_short(n,x)    (*((unsigned short *)n)) = x
  #else
    #define m_big_endian_short(n)                   (get_big_endian_short(n))
    #define m_big_endian_int(n)                     (get_big_endian_int(n))
    #define m_big_endian_float(n)                   (get_big_endian_float(n))
    #define m_big_endian_double(n)                  (get_big_endian_double(n))
    #define m_big_endian_unsigned_short(n)          (get_big_endian_unsigned_short(n))

    #define m_set_big_endian_short(n,x)             set_big_endian_short(n,x)
    #define m_set_big_endian_int(n,x)               set_big_endian_int(n,x)
    #define m_set_big_endian_float(n,x)             set_big_endian_float(n,x)
    #define m_set_big_endian_double(n,x)            set_big_endian_double(n,x)
    #define m_set_big_endian_unsigned_short(n,x)    set_big_endian_unsigned_short(n,x)
  #endif

  #define m_little_endian_short(n)                  (get_little_endian_short(n))
  #define m_little_endian_int(n)                    (get_little_endian_int(n))
  #define m_little_endian_float(n)                  (get_little_endian_float(n))
  #define m_little_endian_double(n)                 (get_little_endian_double(n))
  #define m_little_endian_unsigned_short(n)         (get_little_endian_unsigned_short(n))

  #define m_set_little_endian_short(n,x)            set_little_endian_short(n,x)
  #define m_set_little_endian_int(n,x)              set_little_endian_int(n,x)
  #define m_set_little_endian_float(n,x)            set_little_endian_float(n,x)
  #define m_set_little_endian_double(n,x)           set_little_endian_double(n,x)
  #define m_set_little_endian_unsigned_short(n,x)   set_little_endian_unsigned_short(n,x)


#endif


/* ---------------- arrays ---------------- */
#ifdef CLM

/* these are for C-buffer allocation from Lisp -- in the LONG_INT_P case (where 
 * sizeof(int) != sizeof(int *)), we return an index into a table of pointers, 
 * rather than the pointer itself (the problem is that we need to hang onto to
 * these addresses in Lisp where even 32-bit ints can be hard to find).
 */

#if LONG_INT_P
static int **long_int_p_table = NULL;
static int long_int_p_table_size = 0;

int *delist_ptr(int arr) {return(long_int_p_table[arr]);}

int list_ptr(int *arr) 
{
  int i,loc;
  loc = -1;
  for (i=0;i<long_int_p_table_size;i++) 
    {
      if (long_int_p_table[i] == NULL)
	{
	  loc = i;
	  break;
	}
    }
  if (loc == -1)
    {
      loc = long_int_p_table_size;
      long_int_p_table_size+=16;
      if (long_int_p_table)
	{
	  long_int_p_table = (int **)REALLOC(long_int_p_table,long_int_p_table_size * sizeof(int *));
	  for (i=loc;i<long_int_p_table_size;i++) long_int_p_table[i] = NULL;
	}
      else
	long_int_p_table = (int **)CALLOC(long_int_p_table_size,sizeof(int *));
    }
  long_int_p_table[loc] = arr;
  return(loc);
}

int setarray(int arr_1, int i, int val) {int *arr; arr = delist_ptr(arr_1); arr[i]=val; return(val);}
int getarray(int arr_1, int i) {int *arr; arr = delist_ptr(arr_1); return(arr[i]);}
int incarray(int arr_1, int i, int val) {int *arr; arr = delist_ptr(arr_1); arr[i]+=val; return(arr[i]);}
int makearray(int len) {int *ip; ip = (int *)CALLOC(len,sizeof(int)); return(list_ptr(ip));}

void freearray(int ip_1) 
{
  int *ip; 
  ip = delist_ptr(ip_1); 
  if (ip == NULL) 
    clm_printf("attempt to free invalid pointer!"); 
  else FREE(ip); 
  long_int_p_table[ip_1] = NULL;
}

void cleararray1(int beg, int end, int arr_1) {int *arr; int i; arr = delist_ptr(arr_1); for (i=beg;i<=end;i++) arr[i] = 0;}
void arrblt(int beg, int end, int newbeg, int arr_1) {int *arr; int i,j; arr = delist_ptr(arr_1); for (i=beg,j=newbeg;i>=end;i--,j--) arr[j]=arr[i];}

int absmaxarr(int beg, int end, int arr_1)
{
  int minA,maxA,val,i;
  int *arr;
  arr = delist_ptr(arr_1);
  minA = 0;
  maxA = 0;
  for (i=beg;i<=end;i++)
    {
      val = arr[i];
      if ((val > maxA) || (val < minA))
	{
	  maxA = val;
	  if (maxA < 0) maxA = -maxA;
	  minA = -maxA;
	}
    }
  return(maxA);
}

#else

int setarray(int *arr, int i, int val) {arr[i]=val; return(val);}
int getarray(int *arr, int i) {return(arr[i]);}
int incarray(int *arr, int i, int val) {arr[i]+=val; return(arr[i]);}
int *makearray(int len) {int *ip; ip = (int *)CALLOC(len,sizeof(int)); return(ip);}
void freearray(int *ip) {if (ip == NULL) clm_printf("attempt to free invalid pointer!"); else FREE(ip);}
void cleararray1(int beg, int end, int *arr) {int i; for (i=beg;i<=end;i++) arr[i] = 0;}
void arrblt(int beg, int end, int newbeg, int *arr) {int i,j; for (i=beg,j=newbeg;i>=end;i--,j--) arr[j]=arr[i];}

int absmaxarr(int beg, int end, int *arr)
{
  int minA,maxA,val,i;
  minA = 0;
  maxA = 0;
  for (i=beg;i<=end;i++)
    {
      val = arr[i];
      if ((val > maxA) || (val < minA))
	{
	  maxA = val;
	  if (maxA < 0) maxA = -maxA;
	  minA = -maxA;
	}
    }
  return(maxA);
}
#endif

static float maxamparray(int size, float *arr)
{
  float minA,maxA,val;
  int i;
  minA = 0.0;
  maxA = 0.0;
  for (i=0;i<size;i++)
    {
      val = arr[i];
      if ((val > maxA) || (val < minA))
	{
	  maxA = val;
	  if (maxA < 0.0) maxA = -maxA;
	  minA = -maxA;
	}
    }
  return(maxA);
}

void normarray(int size, float *arr)
{
  float maxa;
  int i;
  maxa=maxamparray(size,arr);
  if ((maxa != 0.0) && (maxa != 1.0))
    {
      maxa=1.0/maxa;
      for (i=0;i<size;i++) arr[i] *= maxa;
    }
}
#endif


/* ---------------- file descriptors ----------------
 *
 * I'm using unbuffered IO here because it is faster on the machines I normally use,
 * and I'm normally doing very large reads/writes (that is, the stuff is self-buffered).
 *
 *   machine                     read/write:              fread/fwrite:             arithmetic: 
 *                               256   512   8192  65536  same sizes                tbl   bigfft sffts
 *
 * NeXT 68040 (32MB):            11575 10514 10256  9943  11951 11923 12358 12259   10478 108122 26622
 * NeXT Turbo (16MB):             8329  7760  6933  6833   9216  8742  9416  9238    7825 121591 19495
 * HP 90MHz Pentium NextStep:    11970 10069  9840  9920  11930 11209 11399 11540    1930  46389  4019
 * Mac 8500 120 MHz PPC MacOS:   21733 15416  5000  2916   9566  9550  9733  9850    <died in memory manager>
 * Mac G3 266 MHz PPC MacOS:      4866  3216  1850  1366   2400  2400  2366  2450     550  12233   700
 * MkLinux G3 266 MHz:             580   462   390   419    640   631   552   500     485  11364   770
 * LinuxPPC G3 266 MHz:            456   385   366   397    489   467   467   487     397  11808   763
 * Mac clone 120 MHz PPC BeOS:    1567   885   725  3392   1015  1000  1114  1161    1092  37212  1167
 * SGI R4600 132 MHz Indy (32MB): 2412  1619   959  1045   1172  1174  1111  1126    1224  30825  3490
 * SGI R5000 150 MHz Indy (32MB): 1067   846   684   737    847   817   734   791     885  25878  1591
 * SGI R5000 180 MHz O2 (64MB):   1359   788   431   446   1919  1944  1891  1885     828  24658  1390
 * Sun Ultra5 270 MHz (128 MB):    981   880   796   827    965  1029   922   903     445  26791   691
 * HP 200 MHz Pentium Linux:       576   492   456   482    615   613   599   592     695  14851   882
 * Asus 266 MHz Pentium II Linux:  475   426   404   406    466   455   467   465     490  13170   595
 * ditto W95:                     1320   660   600   550   2470  2470  2470  2470     990  17410  1540
 * Dell XPSD300 Pentium II Linux:  393   350   325   332    376   369   397   372     414   8793   576
 * 450MHz PC Linux:                263   227   208   217    268   263   274   270     275   6224   506
 *
 * the first 8 numbers are comparing read/write fread/fwrite at various buffer sizes -- CLM uses 65536.
 * the last 3 numbers are comparing table lookup, a huge fft, and a bunch of small ffts.
 * In normal CLM usage, small instruments and mixes are IO bound, so these differences can matter.
 * The reason to use 65536 rather than 8192 is that it allows us to forgo IO completely in
 * many cases -- the output buffer can collect many notes before flushing, etc.
 */

#if defined(SGI) || defined(LINUX) || defined(UW2) || defined(SCO5)
  #define FILE_DESCRIPTORS 400
  #define BASE_FILE_DESCRIPTORS 200
#else
  #define FILE_DESCRIPTORS 128
  #define BASE_FILE_DESCRIPTORS 64
#endif

static int clm_descriptors_ok = 0;
static int *clm_datum_format,*clm_datum_size,*clm_datum_location,*clm_files,*clm_datum_type;
static int clm_files_ready = 0;
static int max_descriptor = 0;

static int rt_ap_out;   /* address of RT audio ports, if any */

void create_descriptors (void)
{
  if (!clm_descriptors_ok)
    {
      clm_descriptors_ok = 1;
      clm_datum_format = (int *)CALLOC(FILE_DESCRIPTORS,sizeof(int));
      clm_datum_size = (int *)CALLOC(FILE_DESCRIPTORS,sizeof(int));
      clm_datum_type = (int *)CALLOC(FILE_DESCRIPTORS,sizeof(int));
      clm_datum_location = (int *)CALLOC(FILE_DESCRIPTORS,sizeof(int));
      clm_files = (int *)CALLOC(BASE_FILE_DESCRIPTORS,sizeof(int));
      if ((clm_datum_format == NULL) || (clm_datum_size == NULL) || (clm_datum_location == NULL) || (clm_files == NULL))
	clm_printf("file descriptor buffer allocation trouble");
      max_descriptor = 0;
    }
}

#ifdef CLM
void set_rt_audio_p (int rt)
{
  rt_ap_out = rt;
}
#endif

static int convert_fd(int n)
{
  if (n<BASE_FILE_DESCRIPTORS)
    return(n);
  else
    {
      int i;
      for (i=0;i<BASE_FILE_DESCRIPTORS;i++)
	{
	  if (clm_files[i] == n) return(i+BASE_FILE_DESCRIPTORS);
	}
      return(-1);
    }
}

static int open_clm_file (int tfd)
{
  int fd;
  if (tfd < BASE_FILE_DESCRIPTORS) return(tfd);
  if (clm_files_ready == 0)
    {
      for (fd=0;fd<BASE_FILE_DESCRIPTORS;fd++) clm_files[fd]=-1;
      clm_files_ready = 1;
    }
  for (fd=0;fd<BASE_FILE_DESCRIPTORS;fd++)
    {
      if (clm_files[fd] == -1)
	{
	  clm_files[fd] = tfd;
	  return(fd+BASE_FILE_DESCRIPTORS);
	}
    }
  return(-1);
}

void open_clm_file_descriptors (int tfd, int df, int ds, int dl)
{ /* transfers header info from functions in header.c back to us for reads here and in merge.c */
  int fd;
  if (!clm_descriptors_ok) return;
  fd = open_clm_file(tfd);
  clm_datum_format[fd] = df;
  clm_datum_size[fd] = ds;
  clm_datum_location[fd] = dl;
  clm_datum_type[fd] = 0;
  if (fd > max_descriptor) max_descriptor = fd;
}

#ifdef CLM
void set_clm_datum_type (int tfd, int type)
{
  int fd;
  if (!clm_descriptors_ok) return;
  fd = convert_fd(tfd);
  clm_datum_type[fd] = type;
}
#endif

void close_clm_file_descriptors(int tfd)
{
  int fd;
  if (!clm_descriptors_ok) return; /* not necessarily an error -- c-close before with-sound etc */
  fd = convert_fd(tfd);
  if (fd >= 0)
    {
      if (fd >= BASE_FILE_DESCRIPTORS)
	clm_files[fd-BASE_FILE_DESCRIPTORS] = -1;
      clm_datum_format[fd]=snd_no_snd;
      clm_datum_type[fd] = 0;
    }
}

void cleanup_clm_file_descriptors(void)
{
  /* error cleanup -- try to find C-opened files that are invisible to lisp and close them */
  int fd,lim;
  if (!clm_descriptors_ok) return;
  lim = BASE_FILE_DESCRIPTORS-1;
  if (max_descriptor < lim) lim = max_descriptor;
  for (fd=0;fd<=lim;fd++)
    if (clm_datum_format[fd] != snd_no_snd) clm_close(fd);
  if ((clm_files_ready) && (max_descriptor > BASE_FILE_DESCRIPTORS))
    {
      lim = max_descriptor - BASE_FILE_DESCRIPTORS;
      if (lim >= BASE_FILE_DESCRIPTORS) lim = BASE_FILE_DESCRIPTORS - 1;
      for (fd=0;fd<=lim;fd++)
	if (clm_files[fd] != -1)
	  clm_close(clm_files[fd]);
    }
}


/* ---------------- open, creat, close ---------------- */

int clm_open_read(char *arg) 
{
#ifdef MACOS
  return(open (arg, O_RDONLY));
#else
  int fd;
  #ifdef WINDOZE
    fd = open (arg, O_RDONLY | O_BINARY);
  #else
    fd = open (arg, O_RDONLY, 0);
  #endif
  return(fd);
#endif
}

int clm_open_write(char *arg)
{
  int fd;
#ifdef MACOS
  if ((fd = open(arg,O_RDWR)) == -1)
  #ifdef MPW_C
    fd = creat(arg);
  #else
    fd = creat(arg, 0);
  #endif
  else
    lseek(fd,0L,SEEK_END);
#else
  #ifdef WINDOZE
    if ((fd = open(arg,O_RDWR | O_BINARY)) == -1)
  #else
    if ((fd = open(arg,O_RDWR,0)) == -1)
  #endif
      {
        fd = creat(arg,0666);  /* equivalent to the new open(arg,O_RDWR | O_CREAT | O_TRUNC, 0666) */
      }
    else
      lseek(fd,0L,SEEK_END);
#endif
  return(fd);
}

int clm_create(char *arg)
{
#ifdef MACOS
  #ifdef MPW_C
    return(creat(arg));
  #else
    return(creat(arg,0));
  #endif
#else
  int fd;
  fd = creat(arg,0666);
  return(fd);
#endif
}

int clm_reopen_write(char *arg)
{
#ifdef MACOS
  return(open(arg,O_RDWR));
#else
  int fd;
  #ifdef WINDOZE
    fd = open(arg,O_RDWR | O_BINARY);
  #else
    fd = open(arg,O_RDWR,0);
  #endif
  return(fd);
#endif
}

void clm_close(int fd)
{
  close_clm_file_descriptors(fd);
  close(fd);
}



/* ---------------- seek ---------------- */

long clm_seek(int tfd, long offset, int origin)
{
  int fd,siz; /* siz = datum size in bytes */
  long loc,true_loc,header_end;
  char *str;
  if (!clm_descriptors_ok) {clm_printf("clm-seek: clm file descriptors not initialized!"); return(-1);}
  if ((tfd == SNDLIB_DAC_CHANNEL) || (tfd == SNDLIB_DAC_REVERB)) return(0);
  fd = convert_fd(tfd);
  if (clm_datum_format[fd] == snd_no_snd) 
    {
      str=(char *)CALLOC(64,sizeof(char));
      sprintf(str,"clm-seek: invalid stream: %d (%d, %d, %d)",fd,tfd,(int)offset,origin);
      clm_printf(str);
      FREE(str);
      return(-1);
    }
  siz = clm_datum_size[fd];
  if ((siz == 2) || (origin != 0))
    return(lseek(tfd,offset,origin));
  else
    {
      header_end = clm_datum_location[fd];
      loc = offset - header_end;
      switch (siz)
	{
	case 1: 
	  true_loc = lseek(tfd,header_end+(loc>>1),origin);
	  /* now pretend we're still in 16-bit land and return where we "actually" are in that region */
	  /* that is, loc (in bytes) = how many (2-byte) samples into the file we want to go, return what we got */
	  return(header_end + ((true_loc - header_end)<<1));
	  break;
	case 3:
	  true_loc = lseek(tfd,header_end+loc+(loc>>1),origin);
	  return(true_loc + ((true_loc - header_end)>>1));
	  break;
	case 4:
	  true_loc = lseek(tfd,header_end+(loc<<1),origin);
	  return(header_end + ((true_loc - header_end)>>1));
	  break;
	case 8:
	  true_loc = lseek(tfd,header_end+(loc<<2),origin);
	  return(header_end + ((true_loc - header_end)>>2));
	  break;
	}
    }
  return(-1);
}

#ifdef CLM
long excl_clm_seek(int tfd, int *offset, int origin)
{
  return(clm_seek(tfd,offset[0]+(offset[1]<<16),origin));
}
#endif

/* ---------------- mulaw/alaw conversions ----------------
 *
 *      x : input signal with max value 32767
 *     mu : compression parameter (mu=255 used for telephony)
 *     y = (32767/log(1+mu))*log(1+mu*abs(x)/32767)*sign(x); -- this isn't right -- typo?
 */

/* from sox g711.c */

#define	SIGN_BIT	(0x80)		/* Sign bit for a A-law byte. */
#define	QUANT_MASK	(0xf)		/* Quantization field mask. */
#define	NSEGS		(8)		/* Number of A-law segments. */
#define	SEG_SHIFT	(4)		/* Left shift for segment number. */
#define	SEG_MASK	(0x70)		/* Segment field mask. */

static short seg_end[8] = {0xFF, 0x1FF, 0x3FF, 0x7FF,  0xFFF, 0x1FFF, 0x3FFF, 0x7FFF};

static int search(int val, short *table, int size)
{
  int i;
  for (i = 0; i < size; i++) {if (val <= *table++) return (i);}
  return (size);
}

static unsigned char to_alaw(int pcm_val)
{
  int mask,seg;
  unsigned char	aval;
  if (pcm_val >= 0) {mask = 0xD5;} else {mask = 0x55; pcm_val = -pcm_val - 8;}
  seg = search(pcm_val, seg_end, 8);
  if (seg >= 8)	return (0x7F ^ mask);
  else 
    {
      aval = seg << SEG_SHIFT;
      if (seg < 2) aval |= (pcm_val >> 4) & QUANT_MASK; else aval |= (pcm_val >> (seg + 3)) & QUANT_MASK;
      return (aval ^ mask);
    }
}

static const int alaw[256] = {
 -5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736, -7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784, 
 -2752, -2624, -3008, -2880, -2240, -2112, -2496, -2368, -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392, 
 -22016, -20992, -24064, -23040, -17920, -16896, -19968, -18944, -30208, -29184, -32256, -31232, -26112, -25088, -28160, -27136, 
 -11008, -10496, -12032, -11520, -8960, -8448, -9984, -9472, -15104, -14592, -16128, -15616, -13056, -12544, -14080, -13568, 
 -344, -328, -376, -360, -280, -264, -312, -296, -472, -456, -504, -488, -408, -392, -440, -424, 
 -88, -72, -120, -104, -24, -8, -56, -40, -216, -200, -248, -232, -152, -136, -184, -168, 
 -1376, -1312, -1504, -1440, -1120, -1056, -1248, -1184, -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696, 
 -688, -656, -752, -720, -560, -528, -624, -592, -944, -912, -1008, -976, -816, -784, -880, -848, 
 5504, 5248, 6016, 5760, 4480, 4224, 4992, 4736, 7552, 7296, 8064, 7808, 6528, 6272, 7040, 6784, 
 2752, 2624, 3008, 2880, 2240, 2112, 2496, 2368, 3776, 3648, 4032, 3904, 3264, 3136, 3520, 3392, 
 22016, 20992, 24064, 23040, 17920, 16896, 19968, 18944, 30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136, 
 11008, 10496, 12032, 11520, 8960, 8448, 9984, 9472, 15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568, 
 344, 328, 376, 360, 280, 264, 312, 296, 472, 456, 504, 488, 408, 392, 440, 424, 
 88, 72, 120, 104, 24, 8, 56, 40, 216, 200, 248, 232, 152, 136, 184, 168, 
 1376, 1312, 1504, 1440, 1120, 1056, 1248, 1184, 1888, 1824, 2016, 1952, 1632, 1568, 1760, 1696, 
 688, 656, 752, 720, 560, 528, 624, 592, 944, 912, 1008, 976, 816, 784, 880, 848
};

#if 0
static int from_alaw(unsigned char a_val)
{
  int t,seg;
  a_val ^= 0x55;
  t = (a_val & QUANT_MASK) << 4;
  seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
  switch (seg) 
    {
    case 0: t += 8; break;
    case 1: t += 0x108; break;
  default:  t += 0x108; t <<= seg - 1;
    }
  return((a_val & SIGN_BIT) ? t : -t);
}
#endif 

#define	BIAS		(0x84)		/* Bias for linear code. */

static unsigned char to_mulaw(int pcm_val)
{
  int mask;
  int seg;
  unsigned char	uval;
  if (pcm_val < 0) {pcm_val = BIAS - pcm_val; mask = 0x7F;} else {pcm_val += BIAS; mask = 0xFF;}
  seg = search(pcm_val, seg_end, 8);
  if (seg >= 8) return (0x7F ^ mask);
  else 
    {
      uval = (seg << 4) | ((pcm_val >> (seg + 3)) & 0xF);
      return (uval ^ mask);
    }
}

/* generated by SNDiMulaw on a NeXT -- see /usr/include/sound/mulaw.h */
static const int mulaw[256] = {
  -32124, -31100, -30076, -29052, -28028, -27004, -25980, -24956, -23932, -22908, -21884, -20860, 
  -19836, -18812, -17788, -16764, -15996, -15484, -14972, -14460, -13948, -13436, -12924, -12412, 
  -11900, -11388, -10876, -10364, -9852, -9340, -8828, -8316, -7932, -7676, -7420, -7164, -6908, 
  -6652, -6396, -6140, -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092, -3900, -3772, -3644, 
  -3516, -3388, -3260, -3132, -3004, -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980, -1884, 
  -1820, -1756, -1692, -1628, -1564, -1500, -1436, -1372, -1308, -1244, -1180, -1116, -1052, -988, 
  -924, -876, -844, -812, -780, -748, -716, -684, -652, -620, -588, -556, -524, -492, -460, -428, 
  -396, -372, -356, -340, -324, -308, -292, -276, -260, -244, -228, -212, -196, -180, -164, -148, 
  -132, -120, -112, -104, -96, -88, -80, -72, -64, -56, -48, -40, -32, -24, -16, -8, 0, 32124, 31100, 
  30076, 29052, 28028, 27004, 25980, 24956, 23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764, 
  15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412, 11900, 11388, 10876, 10364, 9852, 9340, 
  8828, 8316, 7932, 7676, 7420, 7164, 6908, 6652, 6396, 6140, 5884, 5628, 5372, 5116, 4860, 4604, 
  4348, 4092, 3900, 3772, 3644, 3516, 3388, 3260, 3132, 3004, 2876, 2748, 2620, 2492, 2364, 2236, 
  2108, 1980, 1884, 1820, 1756, 1692, 1628, 1564, 1500, 1436, 1372, 1308, 1244, 1180, 1116, 1052, 
  988, 924, 876, 844, 812, 780, 748, 716, 684, 652, 620, 588, 556, 524, 492, 460, 428, 396, 372, 
  356, 340, 324, 308, 292, 276, 260, 244, 228, 212, 196, 180, 164, 148, 132, 120, 112, 104, 96, 
  88, 80, 72, 64, 56, 48, 40, 32, 24, 16, 8, 0};

#if 0
/* in case it's ever needed, here's the mulaw to linear converter from g711.c -- identical to table above */
static int from_mulaw(unsigned char u_val)
{
  int t;
  u_val = ~u_val;
  t = ((u_val & QUANT_MASK) << 3) + BIAS;
  t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;
  return ((u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}
#endif

/* ---------------- read/write buffer allocation ---------------- */

#define BUFLIM 64*1024
static char *charbuf;
static int char_ok = -1;

static void check_charbuf (void)
{
  if (char_ok == -1)
    {
      charbuf = (char *)CALLOC(BUFLIM,sizeof(char)); 
      if (charbuf == NULL) clm_printf("IO buffer allocation trouble");
    }
  char_ok = 0;
}

static int checked_write(int fd, char *buf, int chars)
{
#ifdef CLM
#ifndef MACOS
  long lisp_call(int index);
#endif
#endif
  int bytes,cfd;
  if (fd == SNDLIB_DAC_CHANNEL)
    {
      write_audio(rt_ap_out,buf,chars);
    }
  else
    {
      bytes=write(fd,buf,chars);
      if (bytes != chars) 
	{
	  char *str;
	  if (buf == NULL) clm_printf("IO buffer in checked_write is unallocated (null)!");
	  perror("clm");
	  if (!clm_descriptors_ok) clm_printf("clm file descriptors not initialized!");
	  cfd = convert_fd(fd);
	  if (clm_datum_format[cfd] == snd_no_snd) clm_printf("checked_write called on closed file");
	  str = (char *)CALLOC(256,sizeof(char));
#if LONG_INT_P
	  sprintf(str,"IO write error (%s): %d of %d bytes written for %d from %d (%d %d %d)\n",
		  strerror(errno),
		  bytes,chars,fd,cfd,clm_datum_size[cfd],clm_datum_format[cfd],clm_datum_location[cfd]);
#else
  #ifndef MACOS
	  sprintf(str,"IO write error (%s): %d of %d bytes written for %d from %d (%d %d %d %d)\n",
		  strerror(errno),
		  bytes,chars,fd,cfd,(int)buf,clm_datum_size[cfd],clm_datum_format[cfd],clm_datum_location[cfd]);
  #else
	  sprintf(str,"IO write error: %d of %d bytes written for %d from %d (%d %d %d %d)\n",
		  bytes,chars,fd,cfd,(int)buf,clm_datum_size[cfd],clm_datum_format[cfd],clm_datum_location[cfd]);
  #endif
#endif
	  clm_printf(str);
	  FREE(str);
#ifdef CLM
#ifndef MACOS
	  lisp_call(7);
#endif
#endif
	  return(-1);
	}
    }
  return(0);
}



/* ---------------- read ---------------- */

/* normally we assume a 16-bit fractional part, but sometimes user want 24-bits */
static int shift_24_choice = 0;
#ifdef CLM
int get_shift_24_choice(void) {return(shift_24_choice);}
void set_shift_24_choice(int choice) {shift_24_choice = choice;}
#endif

int clm_read_any(int tfd, int beg, int chans, int nints, int **bufs, int *cm)
{
  int fd;
  int bytes,j,lim,siz,total,leftover,total_read,k,loc,oldloc,siz_chans,buflim;
  unsigned char *jchar;
  int *buffer;
  if (!clm_descriptors_ok) {clm_printf("clm-read: clm file descriptors not initialized!"); return(-1);}
  if (nints <= 0) return(0);
  check_charbuf();
  fd = convert_fd(tfd);
  if (clm_datum_format[fd] == snd_no_snd) clm_printf("read_any called on closed file");
  siz = clm_datum_size[fd];
  siz_chans = siz*chans;
  leftover = (nints*siz_chans);
  k = (BUFLIM) % siz_chans;
  if (k != 0) /* for example, 3 channel output of 1-byte (mulaw) samples will need a mod 3 buffer */
    buflim = (BUFLIM) - k;
  else buflim = BUFLIM;
  total_read = 0;
  loc = beg;
  while (leftover > 0)
    {
      bytes = leftover;
      if (bytes > buflim) {leftover = (bytes-buflim); bytes = buflim;} else leftover = 0;
      total = read(tfd,charbuf,bytes); 
      if (total <= 0) return(total_read);
      lim = (int) (total / siz_chans);  /* this divide must be exact (hence the buflim calc above) */
      total_read += lim;
      oldloc = loc;

      for (k=0;k<chans;k++)
	{
	  if ((cm == NULL) || (cm[k]))
	    {
	      buffer = (int *)(bufs[k]);
	      if (buffer)
		{
		  loc = oldloc;
		  jchar = (unsigned char *)charbuf;
		  jchar += (k*siz);
		  switch (clm_datum_format[fd])
		    {
		    case snd_16_linear:               
		      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) buffer[loc] = (int)m_big_endian_short(jchar); 
		      break;
		    case snd_16_linear_little_endian: 
		      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) buffer[loc] = (int)m_little_endian_short(jchar); 
		      break;
		    case snd_32_linear:              
		      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) buffer[loc] = m_big_endian_int(jchar); 
		      break;
		    case snd_32_linear_little_endian: 
		      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) buffer[loc] = m_little_endian_int(jchar); 
		      break;
		    case snd_8_mulaw:  	              
		      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) buffer[loc] = mulaw[*jchar]; 
		      break;
		    case snd_8_alaw:                  
		      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) buffer[loc] = alaw[*jchar]; 
		      break;
		    case snd_8_linear:                
		      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) buffer[loc] = (int)(((signed char)(*jchar)) << 8); 
		      break;
		    case snd_8_unsigned:     	      
		      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) buffer[loc] = (int) ((((int)(*jchar))-128) << 8); 
		      break;
		    case snd_32_float:
		      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) buffer[loc] = (int) (SNDLIB_SNDFIX*(m_big_endian_float(jchar)));
		      break;
		    case snd_64_double:   
		      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) buffer[loc] = (int) (SNDLIB_SNDFIX*(m_big_endian_double(jchar)));
		      break;
		    case snd_32_float_little_endian:    
		      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) buffer[loc] = (int) (SNDLIB_SNDFIX*(m_little_endian_float(jchar)));
		      break;
		    case snd_64_double_little_endian:   
		      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) buffer[loc] = (int) (SNDLIB_SNDFIX*(m_little_endian_double(jchar)));
		      break;
		    case snd_16_unsigned:   
		      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) buffer[loc] = ((int)(m_big_endian_unsigned_short(jchar)) - 32768);
		      break;
		    case snd_16_unsigned_little_endian:   
		      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) buffer[loc] = ((int)(m_little_endian_unsigned_short(jchar)) - 32768);
		      break;
		    case snd_32_vax_float:   
		      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) buffer[loc] = (int)from_vax_float(jchar);
		      break;
		    case snd_24_linear:
		      if (shift_24_choice == 0)
			{
			  for (j=0;j<lim;j++,loc++,jchar+=siz_chans)
			    buffer[loc] = (int)(((jchar[0]<<24)+(jchar[1]<<16))>>16);
			}
		      else
			{
			  for (j=0;j<lim;j++,loc++,jchar+=siz_chans)
			    buffer[loc] = (int)(((jchar[0]<<24)+(jchar[1]<<16)+(jchar[2]<<8))>>8);
			}
		      break;
		    case snd_24_linear_little_endian:   
		      if (shift_24_choice == 0)
			{
			  for (j=0;j<lim;j++,loc++,jchar+=siz_chans)
			    buffer[loc] = (int)(((jchar[2]<<24)+(jchar[1]<<16))>>16);
			}
		      else
			{
			  for (j=0;j<lim;j++,loc++,jchar+=siz_chans)
			    buffer[loc] = (int)(((jchar[2]<<24)+(jchar[1]<<16)+(jchar[0]<<8))>>8);
			}
		      break;
		    }
		}
	    }
	}
    }
  return(total_read);
}

void clm_read(int fd, int beg, int end, int chans, int **bufs)
{
  int num,rtn,i,k;
  int *buffer;
  num=(end-beg+1);
  rtn=clm_read_any(fd,beg,chans,num,bufs,NULL);
  if (rtn<num) 
    {
      for (k=0;k<chans;k++)
	{
	  buffer=(int *)(bufs[k]);
	  for (i=rtn+beg;i<=end;i++)
	    {
	      buffer[i]=0;
	    }
	}
    }
}

void clm_read_chans(int fd, int beg, int end, int chans, int **bufs, int *cm)
{
  /* an optimization of clm_read -- just reads the desired channels */
  int num,rtn,i,k;
  int *buffer;
  num=(end-beg+1);
  rtn=clm_read_any(fd,beg,chans,num,bufs,cm);
  if (rtn<num) 
    {
      for (k=0;k<chans;k++)
	{
	  if ((cm == NULL) || (cm[k]))
	    {
	      buffer=(int *)(bufs[k]);
	      for (i=rtn+beg;i<=end;i++)
		{
		  buffer[i]=0;
		}
	    }
	}
    }
}


/* ---------------- write ---------------- */

#ifdef WINDOZE
  #undef min
#endif

#define min(x,y)  ((x) < (y) ? (x) : (y))
inline static int ceiling (float x) {int y; y = (int)x; if ((x - y) == 0.0) return(y); return(y+1);}

void clm_write_zeros(int tfd, int num)
{
  int i,k,lim,curnum,fd;
  if (tfd == SNDLIB_DAC_REVERB) return;
  if (!clm_descriptors_ok) {clm_printf("clm-write-zeros: clm file descriptors not initialized!"); return;}
  check_charbuf();
  fd = convert_fd(tfd);
  if (clm_datum_format[fd] == snd_no_snd) {clm_printf("write_zeros called on closed file"); return;}
  if (tfd == -1) {clm_printf("write_zeros to invalid file"); return;}
  lim = num*(clm_datum_size[fd]);
  k=ceiling(lim/(BUFLIM));
  curnum=min(lim,BUFLIM);
  for (i=0;i<curnum;i++) charbuf[i]=0;
  for (i=0;i<=k;i++)
    {
      checked_write(tfd,charbuf,curnum);
      lim=lim-(BUFLIM);
      curnum=min(lim,BUFLIM);
    }
}

#ifdef CLM
#if defined(ACL4) && defined(ALPHA)
/* in this case, the array passed from lisp is a list of table indices */
void clm_write_1(int tfd, int beg, int end, int chans, int *buflist)
{
  int i;
  int **bufs;
  bufs = (int **)CALLOC(chans,sizeof(int *));
  for (i=0;i<chans;i++) bufs[i] = delist_ptr(buflist[i]);
  clm_write(tfd,beg,end,chans,bufs);
  FREE(bufs);
}
void clm_read_1(int fd, int beg, int end, int chans, int *buflist)
{
  int i;
  int **bufs;
  bufs = (int **)CALLOC(chans,sizeof(int *));
  for (i=0;i<chans;i++) bufs[i] = delist_ptr(buflist[i]);
  clm_read(fd,beg,end,chans,bufs);
  FREE(bufs);
}
#endif
#endif

void clm_write(int tfd, int beg, int end, int chans, int **bufs)
{
  int fd;
  int bytes,j,k,lim,siz,leftover,loc,bk,oldloc,buflim,siz_chans,cliploc;
  unsigned char *jchar;
  int *buffer;
  if (tfd == SNDLIB_DAC_REVERB) return;
  if (!clm_descriptors_ok) {clm_printf("clm-write: clm file descriptors not initialized!"); return;}
  check_charbuf();
  fd = convert_fd(tfd);
  if (clm_datum_format[fd] == snd_no_snd) {clm_printf("write called on closed file"); return;}
  if (tfd == -1) {clm_printf("write called on invalid file"); return;}
  siz = clm_datum_size[fd];
  lim=(end-beg+1);
  siz_chans = siz*chans;
  leftover = lim*siz_chans;
  k = (BUFLIM) % siz_chans;
  if (k != 0) 
    buflim = (BUFLIM) - k;
  else buflim = BUFLIM;
  loc = beg;
  while (leftover > 0)
    {
      bytes = leftover;
      if (bytes > buflim) {leftover = (bytes-buflim); bytes = buflim;} else leftover = 0;
      lim = (int)(bytes/siz_chans); /* see note above */
      oldloc = loc;

      for (k=0;k<chans;k++)
	{
	  loc = oldloc;
	  buffer = (int *)(bufs[k]);
	  if (clm_datum_type[fd] == 1)
	    {
	      cliploc = oldloc;
	      for (j=0;j<lim;j++,cliploc++)
		{
		  if (buffer[cliploc] > 32767)
		    buffer[cliploc] = 32767;
		  else
		    if (buffer[cliploc] < -32768)
		      buffer[cliploc] = -32768;
		}
	    }
	  jchar = (unsigned char *)charbuf;
	  jchar += (k*siz);
	  switch (clm_datum_format[fd])
	    {
	    case snd_16_linear: 
	      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) m_set_big_endian_short(jchar,(short)(buffer[loc]));
	      break;
	    case snd_16_linear_little_endian:   
	      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) m_set_little_endian_short(jchar,(short)(buffer[loc]));
	      break;
	    case snd_32_linear:   
	      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) m_set_big_endian_int(jchar,buffer[loc]);
	      break;
	    case snd_32_linear_little_endian:   
	      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) m_set_little_endian_int(jchar,buffer[loc]);
	      break;
	    case snd_8_mulaw:     
	      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) (*jchar) = to_mulaw(buffer[loc]);
	      break;
	    case snd_8_alaw:      
	      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) (*jchar) = to_alaw(buffer[loc]);
	      break;
	    case snd_8_linear:    
	      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) (*((signed char *)jchar)) = ((buffer[loc])>>8);
	      break;
	    case snd_8_unsigned:  
	      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) (*jchar) = ((buffer[loc])>>8)+128;
	      break;
	    case snd_32_float:    
	      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) m_set_big_endian_float(jchar,(SNDLIB_SNDFLT * (buffer[loc])));
	      break;
	    case snd_32_float_little_endian:    
	      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) m_set_little_endian_float(jchar,(SNDLIB_SNDFLT * (buffer[loc])));
	      break;
	    case snd_64_double:
	      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) m_set_big_endian_double(jchar,(SNDLIB_SNDFLT * (buffer[loc])));
	      break;
	    case snd_64_double_little_endian:   
	      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) m_set_little_endian_double(jchar,(SNDLIB_SNDFLT * (buffer[loc])));
	      break;
	    case snd_16_unsigned: 
	      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) m_set_big_endian_unsigned_short(jchar,(short)(buffer[loc] + 32768));
	      break;
	    case snd_16_unsigned_little_endian: 
	      for (j=0;j<lim;j++,loc++,jchar+=siz_chans) m_set_little_endian_unsigned_short(jchar,(short)(buffer[loc] + 32768));
	      break;
	    case snd_24_linear:   
	      bk=(k*3);
	      if (shift_24_choice == 0)
		{
		  for (j=0;j<lim;j++,loc++,bk+=(chans*3)) 
		    {
		      charbuf[bk]=((buffer[loc])>>8); 
		      charbuf[bk+1]=((buffer[loc])&0xFF); 
		      charbuf[bk+2]=0;	
		    }
		}
	      else
		{
		  for (j=0;j<lim;j++,loc++,bk+=(chans*3)) 
		    {
		      charbuf[bk]=((buffer[loc])>>16); 
		      charbuf[bk+1]=((buffer[loc])>>8); 
		      charbuf[bk+2]=((buffer[loc])&0xFF); 
		    }
		}
	      break;
	    case snd_24_linear_little_endian:   
	      bk=(k*3);
	      if (shift_24_choice == 0)
		{
		  for (j=0;j<lim;j++,loc++,bk+=(chans*3))
		    {
		      charbuf[bk+2]=((buffer[loc])>>8); 
		      charbuf[bk+1]=((buffer[loc])&0xFF); 
		      charbuf[bk]=0;    
		    }
		}
	      else
		{
		  for (j=0;j<lim;j++,loc++,bk+=(chans*3))
		    {
		      charbuf[bk+2]=((buffer[loc])>>16); 
		      charbuf[bk+1]=((buffer[loc])>>8); 
		      charbuf[bk]=((buffer[loc])&0xFF); 
		    }
		}
	      break;
	    }
	}
      checked_write(tfd,charbuf,bytes);
    }
}



/* -------------------------------- floating point data files -------------------------------- */
#ifdef CLM

int clm_read_floats(int fd,int n,float *arr)
{
  char *buf;
  int bytes;
  buf = (char *)arr;
  bytes=read(fd,buf,n*sizeof(float));
  return(bytes/sizeof(float));
}

int clm_read_ints(int fd,int n,int *arr)
{
  char *buf;
  int bytes;
  buf = (char *)arr;
  bytes=read(fd,buf,n*sizeof(int));
  return(bytes/sizeof(int));
}

int clm_read_bytes(int fd,int n,char *arr)
{
  return(read(fd,arr,n));
}

int clm_write_bytes(int fd,int n,char *arr)
{
  return(write(fd,arr,n));
}

int clm_write_floats(int fd,int n,float *arr)
{
  char *buf;
  int bytes;
  buf = (char *)arr;
  bytes=write(fd,buf,n*sizeof(float));
  return(bytes/sizeof(float));
}

int clm_read_swapped_floats(int fd,int n,float *arr)
{
  char *buf;
  int bytes,i;
  char tmp;
  buf = (char *)arr;
  bytes=read(fd,buf,n*4);
  for (i=0;i<bytes;i+=4)
    {tmp=buf[i]; buf[i]=buf[i+3]; buf[i+3]=tmp; tmp=buf[i+1]; buf[i+1]=buf[i+2]; buf[i+2]=tmp;}
  return(bytes>>2);
}

int clm_read_swapped_ints(int fd,int n,int *arr)
{
  char *buf;
  int bytes,i;
  char tmp;
  buf = (char *)arr;
  bytes=read(fd,buf,n*4);
  for (i=0;i<bytes;i+=4)
    {tmp=buf[i]; buf[i]=buf[i+3]; buf[i+3]=tmp; tmp=buf[i+1]; buf[i+1]=buf[i+2]; buf[i+2]=tmp;}
  return(bytes>>2);
}

void clm_seek_floats(int fd,int n)
{
  lseek(fd,n*sizeof(float),SEEK_SET);
}

void clm_seek_bytes(int fd,int n)
{
  lseek(fd,n,SEEK_SET);
}

#ifndef MACOS
sigfnc *clm_signal(int signo, sigfnc *fnc) {return(signal(signo,fnc));}
#endif
#endif

void float_sound(char *charbuf, int samps, int charbuf_format, float *buffer)
{
  /* translate whatever is in charbuf to 32-bit floats still interleaved */
  int j,siz;
  unsigned char *jchar;
  siz = c_snd_datum_size(charbuf_format);
  jchar = (unsigned char *)charbuf;
  switch (charbuf_format)
    {
    case snd_16_linear:   
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = (float)(m_big_endian_short(jchar)); 
      break;
    case snd_16_linear_little_endian:   
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = (float)(m_little_endian_short(jchar)); 
      break;
    case snd_32_linear:   
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = (float)(m_big_endian_int(jchar));
      break;
    case snd_32_linear_little_endian:   
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = (float)(m_little_endian_int(jchar));
      break;
    case snd_8_mulaw:
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = (float)(mulaw[*jchar]);
      break;
    case snd_8_alaw:      
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = (float)(alaw[*jchar]);
      break;
    case snd_8_linear:
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = (float)((int)((*((signed char *)jchar)) << 8));
      break;
    case snd_8_unsigned:  
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = (float)((int) ((((int)(*jchar))-128) << 8));
      break;
    case snd_24_linear:
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = (float)((int)(((jchar[0]<<24)+(jchar[1]<<16))>>16));
      break;
    case snd_24_linear_little_endian:   
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = (float)((int)(((jchar[2]<<24)+(jchar[1]<<16))>>16));
      break;
    case snd_32_float:
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = m_big_endian_float(jchar);
      break;
    case snd_64_double:   
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = (float)(m_big_endian_double(jchar));
      break;
    case snd_32_float_little_endian:    
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = m_little_endian_float(jchar);
      break;
    case snd_64_double_little_endian:   
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = (float)(m_little_endian_double(jchar));
      break;
    case snd_16_unsigned:   
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = (float)(((int)(m_big_endian_unsigned_short(jchar)) - 32768));
      break;
    case snd_32_vax_float:   
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = (float)((int)from_vax_float(jchar));
      break;
    case snd_16_unsigned_little_endian:   
      for (j=0;j<samps;j++,jchar+=siz) buffer[j] = (float)((int)(m_little_endian_unsigned_short(jchar)) - 32768);
      break;
    }
}

#ifdef CLM
/* originally part of clmnet.c, but requires endian handlers and is easier to deal with on the Mac if it's in this file */
int net_mix(int fd, int loc, char *buf1, char *buf2, int bytes)
{
#if defined(SNDLIB_LITTLE_ENDIAN) || defined(SUN)
  unsigned char*b1,*b2;
#else
  short *dat1,*dat2;
#endif  
  int i,lim,rtn;
  lim = bytes>>1;
  lseek(fd,loc,SEEK_SET);
  rtn = read(fd,buf1,bytes);
  if (rtn < bytes)
    {
      for (i=rtn;i<bytes;i++) buf1[i]=buf2[i];
      lim = rtn>>1;
    }
  lseek(fd,loc,SEEK_SET);
#if defined(SNDLIB_LITTLE_ENDIAN) || defined(SUN)
  /* all intermediate results are written as big-endian shorts (NeXT output) */
  b1 = (unsigned char *)buf1;
  b2 = (unsigned char *)buf2;
  for (i=0;i<lim;i++,b1+=2,b2+=2) set_big_endian_short(b1,(short)(get_big_endian_short(b1) + get_big_endian_short(b2)));
#else
  dat1 = (short *)buf1;
  dat2 = (short *)buf2;
  for (i=0;i<lim;i++) dat1[i] += dat2[i];
#endif
  write(fd,buf1,bytes);
  return(0);
}
#endif

int unshort_sound(short *in_buf, int samps, int new_format, char *out_buf)
{
  int j,siz;
  unsigned char *jchar;
  siz = c_snd_datum_size(new_format);
  jchar = (unsigned char *)out_buf;
  switch (new_format)
    {
    case snd_16_linear:   
      for (j=0;j<samps;j++,jchar+=siz) m_set_big_endian_short(jchar,in_buf[j]);
      break;
    case snd_16_linear_little_endian:   
      for (j=0;j<samps;j++,jchar+=siz) m_set_little_endian_short(jchar,in_buf[j]);
      break;
    case snd_32_linear:   
      for (j=0;j<samps;j++,jchar+=siz) m_set_big_endian_int(jchar,(int)in_buf[j]);
      break;
    case snd_32_linear_little_endian:   
      for (j=0;j<samps;j++,jchar+=siz) m_set_little_endian_int(jchar,(int)in_buf[j]);
      break;
    case snd_8_mulaw:     
      for (j=0;j<samps;j++,jchar+=siz) (*jchar) = to_mulaw(in_buf[j]);
      break;
    case snd_8_alaw:      
      for (j=0;j<samps;j++,jchar+=siz) (*jchar) = to_alaw(in_buf[j]);
      break;
    case snd_8_linear:    
      for (j=0;j<samps;j++,jchar+=siz) (*((signed char *)jchar)) = ((in_buf[j])>>8);
      break;
    case snd_8_unsigned:  
      for (j=0;j<samps;j++,jchar+=siz) (*jchar) = ((in_buf[j])>>8)+128;
      break;
    case snd_32_float:    
      for (j=0;j<samps;j++,jchar+=siz) m_set_big_endian_float(jchar,(SNDLIB_SNDFLT * (in_buf[j])));
      break;
    case snd_32_float_little_endian:    
      for (j=0;j<samps;j++,jchar+=siz) m_set_little_endian_float(jchar,(SNDLIB_SNDFLT * (in_buf[j])));
      break;
    default: return(0); break;
    }
  return(samps*siz);
}

#ifdef CLM
void reset_io_c(void) 
{
  clm_descriptors_ok = 0; 
  clm_files_ready = 0;
  char_ok = -1;
#if LONG_INT_P
  long_int_p_table = NULL;
  long_int_p_table_size = 0;
#endif
}
#endif
