/* readers/writers for various sound file headers
 *
 * TODO: avi, asf, and soundfont code seriously incomplete
 *       avr doesn't match actual files
 *       quicktime is just a guess
 *       esignal not tackled yet
 *       check resource info on Mac (for SoundEdit)
 *
 * --------------------------------
 * int c_read_header (char *name): reads file's header
 * int c_write_header (char *name, int type, int in_srate, int in_chans, int loc, int size, int format, char *comment, int len): writes file's header
 * int c_update_header (char *name, int type, int size, int srate, int format, int chans, int loc): update file's header (unset fields should be 0)
 * void create_header_buffer (void): initialize (allocate) various buffers -- should be called before using c_read_header and others
 *
 *   Once c_read_header has been called, the data in it can be accessed through:
 *
 *        int c_snd_header_data_size (void):           samples 
 *        int c_snd_header_data_location (void)        location of data (bytes)
 *        int c_snd_header_chans (void)                channels
 *        int c_snd_header_srate (void)                srate
 *        int c_snd_header_type (void)                 header type (i.e. aiff, wave, etc)  (see sndlib.h)
 *        int c_snd_header_format (void)               data format (see sndlib.h)
 *        int c_snd_header_distributed (void)          true if header info is scattered around in the file
 *        int c_snd_header_comment_start (void)        comment start location (if any) (bytes)
 *        int c_snd_header_comment_end (void)          comment end location
 *        int c_snd_header_aux_comment_start (int n)   if multiple comments, nth start location
 *        int c_snd_header_aux_comment_end (int n)     if multiple comments, nth end location
 *        int c_snd_header_type_specifier (void)       original (header-specific) type ID
 *        int c_snd_header_bits_per_sample (void)      sample width in bits
 *        int c_true_file_length (void)                true (lseek) file length
 *        int c_snd_header_datum_size (void)           sample width in bytes
 * --------------------------------
 *
 *   "Linear" below means 2's complement integer.
 *
 * Currently supported read/write (in standard data formats):                         
 *      NeXT/Sun/DEC/AFsp
 *      AIFF/AIFC
 *      RIFF (microsoft wave)
 *      IRCAM (old style)
 *      NIST-sphere
 *      no header
 *
 * Currently supported read-only (in selected data formats):
 *      8SVX (IFF), IRCAM Vax float, EBICSF, INRS, ESPS, SPPACK, ADC (OGI), AVR, VOC,
 *      Sound Tools, Turtle Beach SMP, SoundFont 2.0, Sound Designer I and II, PSION alaw, MAUD, 
 *      Tandy DeskMate new and old style, Gravis Ultrasound, Comdisco SPW, Goldwave sample, OMF,
 *      Sonic Foundry, SBStudio II, Delusion digital, Digiplayer ST3, Farandole Composer WaveSample,
 *      Ultratracker WaveSample, Sample Dump exchange, Yamaha SY85 and SY99 (buggy), Yamaha TX16, 
 *      Covox v8, SPL, AVI, Kurzweil 2000
 *
 * for a few of these I'm still trying to get documentation -- best sources of info
 * are ftp.cwi.nl:pub/audio (info files), the AFsp sources, and the SOX sources.
 * sox and gsm are at ftp.cwi.nl, AFsp is from kabal@Polaris.EE.McGill.CA (Peter Kabal) as
 * ftp.TSP.EE.McGill.CA:/pub/AFsp/AFsp-V3R2.tar.Z.  The Sound Designer formats are described
 * in the "Developer Documentation" from Digidesign.  Other useful sources can be found at
 * ftp.x.org:/contrib/audio/nas, svr-ftp.eng.cam.ac.uk:/comp.speech/tools, and
 * at http://www.wotsit.demon.co.uk/music.htm.  I put many of my test cases in 
 * ccrma-ftp.stanford.edu:/pub/Lisp/sf.tar.gz.  The RIFF format is described in the 
 * Microsoft Multimedia Programmer's Reference Manual at ftp.microsoft.com:/SoftLib/MSLFILES/MDRK.EXE.
 * AVI format is described in http://www.rahul.net/jfm/avi.html.
 *
 * The main problem with compressed sound files is that you can't do reliable
 * random access to the data, can't easily read backwards, and most of the compression
 * schemes are proprietary (and appalling), but to translate Mus10/Sam, HCOM, IEEE text, 
 * IBM CVSD, MIDI sample dumps, various adpcm cases, NIST shortpack files, and AVI see snd-trans.c 
 * in the sound editor (snd.tar.gz).
 *
 * If anyone has information on any other header or data formats, I would be most interested in it,
 * but only if it can be included in this file.
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
#include <string.h>

#if (defined(NEXT) || (defined(HAVE_LIBC_H) && (!defined(HAVE_UNISTD_H))))
  #include <libc.h>
#else
  #if (!(defined(_MSC_VER))) && (!(defined(MPW_C)))
    #include <unistd.h>
  #endif
#endif

#include "sndlib.h"

static int hdrbuf_is_inited = 0;

#define HDRBUFSIZ 256
#ifndef MACOS
  static unsigned char *hdrbuf;
#else  
  static char *hdrbuf;
#endif

#define INITIAL_READ_SIZE 32

/* AIFF files can have any number of ANNO chunks, so we'll grab at least 4 of them */
/* if AUX_COMMENTS is changed, remember to change aux-comments in headers.lisp */
#define AUX_COMMENTS 4
static int *aux_comment_start = NULL, *aux_comment_end = NULL;

#define LOOPS 2
static int *loop_modes = NULL,*loop_starts = NULL,*loop_ends = NULL;
static int markers = 0;
static int *marker_ids = NULL,*marker_positions = NULL;

#ifdef CLM
void reset_headers_c(void) 
{
  hdrbuf_is_inited = 0; 
  markers = 0;
}
#endif

void create_header_buffer (void)
{
  if (hdrbuf_is_inited == 0)
    {
      hdrbuf_is_inited = 1;
#ifndef MACOS
      hdrbuf = (unsigned char *)CALLOC(HDRBUFSIZ,sizeof(unsigned char));
#else
      hdrbuf = (char *)CALLOC(HDRBUFSIZ,sizeof(unsigned char));
#endif
      aux_comment_start = (int *)CALLOC(AUX_COMMENTS,sizeof(int));
      aux_comment_end = (int *)CALLOC(AUX_COMMENTS,sizeof(int));
      if ((hdrbuf == NULL) || (aux_comment_start == NULL) || (aux_comment_end == NULL))
	clm_printf("header buffer allocation trouble");
      loop_modes = (int *)CALLOC(LOOPS,sizeof(int));
      loop_starts = (int *)CALLOC(LOOPS,sizeof(int));
      loop_ends = (int *)CALLOC(LOOPS,sizeof(int));
    }
}


static const unsigned char I_DSND[4] = {'.','s','n','d'};  /* NeXT/Sun/Dec/SGI/AFsp first word */
static const unsigned char I_FORM[4] = {'F','O','R','M'};  /* AIFF first word */
static const unsigned char I_AIFF[4] = {'A','I','F','F'};  /* AIFF second word */
static const unsigned char I_AIFC[4] = {'A','I','F','C'};  /* ditto but might be compressed data */
static const unsigned char I_COMM[4] = {'C','O','M','M'};
static const unsigned char I_COMT[4] = {'C','O','M','T'};
static const unsigned char I_INFO[4] = {'I','N','F','O'};
static const unsigned char I_INST[4] = {'I','N','S','T'};
static const unsigned char I_inst[4] = {'i','n','s','t'};  /* RIFF wants lower case, just to be different */
static const unsigned char I_MARK[4] = {'M','A','R','K'};
static const unsigned char I_SSND[4] = {'S','S','N','D'};
static const unsigned char I_FVER[4] = {'F','V','E','R'};
static const unsigned char I_NONE[4] = {'N','O','N','E'};
static const unsigned char I_ULAW[4] = {'U','L','A','W'};  /* AIFC compression types that we can handle */
static const unsigned char I_ulaw[4] = {'u','l','a','w'};  /* or maybe it's lowercase (Apple) ... */
static const unsigned char I_ima4[4] = {'i','m','a','4'};  /* AIFC IMA adpcm apparently */
static const unsigned char I_raw_[4] = {'r','a','w',' '};  /* AIFC offset binary OS 8.5 (others are 'MAC3' 'MAC6' 'cdx4' 'cdx2' 'str4') */
static const unsigned char I_sowt[4] = {'s','o','w','t'};  /* AIFC little endian? */
static const unsigned char I_fl32[4] = {'f','l','3','2'};  /* AIFC 32-bit float? */
static const unsigned char I_fl64[4] = {'f','l','6','4'};  /* AIFC 64-bit float? */
static const unsigned char I_twos[4] = {'t','w','o','s'};  /* AIFC big endian? */
static const unsigned char I_ALAW[4] = {'A','L','A','W'};
static const unsigned char I_alaw[4] = {'a','l','a','w'};  /* apple */
static const unsigned char I_APPL[4] = {'A','P','P','L'};
static const unsigned char I_CLM_[4] = {'C','L','M',' '};  /* I hereby claim this AIFF chunk name */
static const unsigned char I_RIFF[4] = {'R','I','F','F'};  /* RIFF first word */
static const unsigned char I_RIFX[4] = {'R','I','F','X'};  /* RIFX first word (big-endian RIFF file) */
static const unsigned char I_WAVE[4] = {'W','A','V','E'};
static const unsigned char I_fmt_[4] = {'f','m','t',' '};
static const unsigned char I_data[4] = {'d','a','t','a'};
static const unsigned char I_fact[4] = {'f','a','c','t'};  /* used by compressed RIFF files */
static const unsigned char I_clm_[4] = {'c','l','m',' '};
static const unsigned char I_NIST[4] = {'N','I','S','T'};  /* first word of NIST SPHERE files */
static const unsigned char I_8SVX[4] = {'8','S','V','X'};  /* AIFF other choice */
static const unsigned char I_dcdt[4] = {'.','c','d','t'};  /* no-ffi CLM data file */
static const unsigned char I_VOC0[4] = {'C','r','e','a'};  /* Actual text is "Creative Voice File" */
static const unsigned char I_VOC1[4] = {'t','i','v','e'};
static const unsigned char I_SOUN[4] = {'S','O','U','N'};  /* Sound Tools first word="SOUND" -- not unique as SMP files start with "SOUND SAMPLE" */
static const unsigned char I_SMP1[4] = {'D',' ','S','A'};
static const unsigned char I_SMP2[4] = {'M','P','L','E'};
static const unsigned char I_BODY[4] = {'B','O','D','Y'};  /* next 4 for 8svx chunk names */
static const unsigned char I_VHDR[4] = {'V','H','D','R'};
static const unsigned char I_CHAN[4] = {'C','H','A','N'};
static const unsigned char I_ANNO[4] = {'A','N','N','O'};
static const unsigned char I_NAME[4] = {'N','A','M','E'};
static const unsigned char I_AVR_[4] = {'2','B','I','T'};  /* first word of AVR files */
static const unsigned char I_HCOM[4] = {'H','C','O','M'};
static const unsigned char I_FSSD[4] = {'F','S','S','D'};
static const unsigned char I_SPIB[4] = {'%','/','/','\n'}; /* first word of IEEE spib text sound files */
static const unsigned char I_S___[4] = {'%','-','-','-'};  /* first word of other IEEE spib text sound files */
static const unsigned char I_ALaw[4] = {'A','L','a','w'};  /* first word of PSION alaw files */
static const unsigned char I_Soun[4] = {'S','o','u','n'};  /* second */
static const unsigned char I_MAUD[4] = {'M','A','U','D'};  /* MAUD specialization of AIFF */
static const unsigned char I_MHDR[4] = {'M','H','D','R'};
static const unsigned char I_MDAT[4] = {'M','D','A','T'};
static const unsigned char I_mdat[4] = {'m','d','a','t'};  /* quicktime */
static const unsigned char I_AFsp[4] = {'A','F','s','p'};
static const unsigned char I_MThd[4] = {'M','T','h','d'};  /* sigh -- the M word */
static const unsigned char I_DVSM[4] = {'D','V','S','M'};  /* first word of DVSM files */
static const unsigned char I_DECN[4] = {'.','s','d','\0'}; /* first word of DEC files (?) */
static const unsigned char I_Esig[4] = {'E','s','i','g'};  /* first word of Esignal files */
static const unsigned char I_nalc[4] = {'n','a','l','\n'}; /* second word of Esignal files */
static const unsigned char I_sfbk[4] = {'s','f','b','k'};  /* SoundFont 2.0 */
static const unsigned char I_sdta[4] = {'s','d','t','a'};
static const unsigned char I_shdr[4] = {'s','h','d','r'};
static const unsigned char I_smpl[4] = {'s','m','p','l'};
static const unsigned char I_pdta[4] = {'p','d','t','a'};
static const unsigned char I_LIST[4] = {'L','I','S','T'};
static const unsigned char I_GF1P[4] = {'G','F','1','P'};  /* first word of Gravis Ultrsound patch files */
static const unsigned char I_ATCH[4] = {'A','T','C','H'};  /* second word */
static const unsigned char I_DSIG[4] = {'$','S','I','G'};  /* first word of Comdisco SPW file */
static const unsigned char I_NAL_[4] = {'N','A','L','_'};  /* second word */
static const unsigned char I_GOLD[4] = {'G','O','L','D'};  /* first word Goldwave(?) sample file */
static const unsigned char I__WAV[4] = {' ','S','A','M'};  /* second word */
static const unsigned char I_SRFS[4] = {'S','R','F','S'};  /* first word Sonic Resource Foundry file(?) */
static const unsigned char I_Diam[4] = {'D','i','a','m'};  /* first word DiamondWare file */
static const unsigned char I_ondW[4] = {'o','n','d','W'};  /* second word */
static const unsigned char I_Drat[4] = {'.','r','a','\xfd'};  /* first word real audio file */
static const unsigned char I_CSRE[4] = {'C','S','R','E'};  /* adf first word -- second starts with "40" */
static const unsigned char I_SND_[4] = {'S','N','D',' '};  /* SBStudio II */
static const unsigned char I_SNIN[4] = {'S','N','I','N'};
static const unsigned char I_SNDT[4] = {'S','N','D','T'};
static const unsigned char I_DDSF[4] = {'D','D','S','F'};  /* Delusion Digital Sound File */
static const unsigned char I_FSMt[4] = {'F','S','M','\376'};  /* Farandole Composer WaveSample */
static const unsigned char I_SDXc[4] = {'S','D','X',':'};  /* Sample dump exchange format */
static const unsigned char I_UWFD[4] = {'U','W','F','D'};  /* Ultratracker Wavesample */
static const unsigned char I_LM89[4] = {'L','M','8','9'};  /* Yamaha TX-16 */
static const unsigned char I_SY80[4] = {'S','Y','8','0'};  /* Yamaha SY-99 */
static const unsigned char I_SY85[4] = {'S','Y','8','5'};  /* Yamaha SY-85 */
static const unsigned char I_SCRS[4] = {'S','C','R','S'};  /* Digiplayer ST3 */
static const unsigned char I_covox[4] = {'\377','\125','\377','\252'};
static const unsigned char I_DSPL[4] = {'D','S','P','L'};  /* Digitracker SPL (now obsolete) */
static const unsigned char I_AVI_[4] = {'A','V','I',' '};  /* RIFF AVI */
static const unsigned char I_strf[4] = {'s','t','r','f'};  
static const unsigned char I_movi[4] = {'m','o','v','i'};  
static const unsigned char I_PRAM[4] = {'P','R','A','M'};  /* Kurzweil 2000 */
static const unsigned char I_ones[4] = {'\377','\377','\377','\377'};
static const unsigned char I_zeros[4] = {'\0','\0','\0','\0'};
static const unsigned char I_asf0[4] = {'\321','\051','\342','\326'};
static const unsigned char I_asf1[4] = {'\332','\065','\321','\021'};
static const unsigned char I_asf2[4] = {'\220','\064','\000','\240'};
static const unsigned char I_asf3[4] = {'\311','\003','\111','\276'};

/* .glt and .shp -> Perry Cook's SPASM data files */

#define I_IRCAM_VAX  0x0001a364
#define I_IRCAM_SUN  0x0002a364
#define I_IRCAM_MIPS 0x0003a364
#define I_IRCAM_NEXT  0x0004a364

#define NINRS	7
static const unsigned int I_INRS[NINRS]={0xcb460020,0xd0465555,0xfa460000,0x1c470040,0x3b470080,0x7a470000,0x9c470040};

static int data_size=0, data_location=0, srate=0, chans=0, header_type=0, sound_format=0, original_sound_format=0, true_file_length=0;
static int comment_start=0, comment_end=0, header_distributed=0, type_specifier=0, bits_per_sample=0, fact_samples=0, block_align=0;
static int base_detune = 0, base_note = 0;

int c_snd_header_data_size (void) {return(data_size);}
int c_snd_header_data_location (void) {return(data_location);}
int c_snd_header_chans (void) {return(chans);}
int c_snd_header_srate (void) {return(srate);}
int c_snd_header_type (void) {return(header_type);}
int c_snd_header_format (void) {return(sound_format);}
int c_snd_header_distributed (void) {return(header_distributed);}
int c_snd_header_comment_start (void) {return(comment_start);}
int c_snd_header_comment_end (void) {return(comment_end);}
int c_snd_header_aux_comment_start (int n) {return(aux_comment_start[n]);}
int c_snd_header_aux_comment_end (int n) {return(aux_comment_end[n]);}
int c_snd_header_type_specifier (void) {return(type_specifier);}
int c_snd_header_bits_per_sample (void) {return(bits_per_sample);}
int c_snd_header_fact_samples (void) {return(fact_samples);}
int c_snd_header_block_align (void) {return(block_align);}
int c_true_file_length (void) {return(true_file_length);}
int c_snd_header_original_format (void) {return(original_sound_format);}
int c_snd_header_loop_mode(int which) {if (loop_modes) return(loop_modes[which]); else return(-1);}
int c_snd_header_loop_start(int which) {if (loop_starts) return(loop_starts[which]); else return(-1);}
int c_snd_header_loop_end(int which) {if (loop_ends) return(loop_ends[which]); else return(-1);}
int c_snd_header_mark_position(int id) {int i; for (i=0;i<markers;i++) {if (marker_ids[i] == id) return(marker_positions[i]);} return(-1);}
int c_snd_header_base_detune(void) {return(base_detune);}
int c_snd_header_base_note(void) {return(base_note);}

int c_snd_datum_size (int format)
{
  switch (format)
    {
    case snd_8_linear: return(1); break;
    case snd_16_linear: return(2); break;
    case snd_8_unsigned: return(1); break;
    case snd_8_mulaw: return(1); break;
    case snd_8_alaw: return(1); break;
    case snd_32_linear: return(4); break;
    case snd_32_float: return(4); break;
    case snd_24_linear: return(3); break;
    case snd_64_double: return(8); break;
    case snd_16_linear_little_endian: return(2); break;
    case snd_32_linear_little_endian: return(4); break;
    case snd_32_float_little_endian: return(4); break;
    case snd_64_double_little_endian: return(8); break;
    case snd_24_linear_little_endian: return(3); break;
    case snd_16_unsigned: return(2); break;
    case snd_16_unsigned_little_endian: return(2); break;
    case snd_32_vax_float: return(4); break;
    default: return(1); break; /* we divide by this number, so 0 is not safe */
    }
}

void c_set_snd_header (int in_srate, int in_chans, int in_format) 
{
  srate = in_srate; 
  chans = in_chans; 
  sound_format = in_format;
  data_size = c_snd_samples(in_format,true_file_length);
}

int c_snd_header_datum_size (void) {return(c_snd_datum_size(sound_format));}
int c_snd_bytes (int format, int size) {return(size*(c_snd_datum_size(format)));}
int c_snd_samples (int format, int size) {return((int)(size/(c_snd_datum_size(format))));}


static int equal_big_or_little_endian(unsigned char *n1, const unsigned int n2)
{
  return((get_big_endian_unsigned_int(n1) == n2) || (get_little_endian_unsigned_int(n1) == n2));
}

static short big_or_little_endian_short (unsigned char *n, int little)
{
  if (little) return(get_little_endian_short(n));
  return(get_big_endian_short(n));
}

static int big_or_little_endian_int (unsigned char *n, int little)
{
  if (little) return(get_little_endian_int(n));
  return(get_big_endian_int(n));
}

static float big_or_little_endian_float (unsigned char *n, int little)
{
  if (little) return(get_little_endian_float(n));
  return(get_big_endian_float(n));
}

int match_four_chars(unsigned char *head, const unsigned char *match)
{ 
  int i;
  for (i=0;i<4;i++) if (head[i] != match[i]) return(0);
  return(1);
}
  
static void write_four_chars(unsigned char *head, const unsigned char *match)
{
  int i;
  for (i=0;i<4;i++) head[i] = match[i];
}


static void read_bicsf_header (int chan);


/* ------------------------------------ NeXT (or Sun) -------------------------------- 
 * 
 *   0:  ".snd"
 *   4:  data_location (bytes) (not necessarily word aligned on Sun)
 *   8:  data_size (bytes) -- sometimes incorrect ("advisory")
 *   12: data format indicator -- see below
 *   16: srate (int)
 *   20: chans
 *   24: comment start
 *   
 * in an AFsp file, the first 4 bytes of the comment are "AFsp",
 * see headers.lisp for readers/writers of AFsp fields
 * for bicsf, the integer at 28 is 107364 or 107415
 *
 * on NeXTStep, always big-endian.  ".snd"==0x2e736e64 on big-endian machines.
 *
 * formats are: 
 * 0 unspecified, 1 mulaw_8, 2 linear_8, 3 linear_16, 4 linear_24, 5 linear_32, 6 float,
 * 7 double, 8 indirect, 9 nested, 10 dsp_core, 11 dsp_data_8, 12 dsp_data_16, 13 dsp_data_24,
 * 14 dsp_data_32, 16 display, 17 mulaw_squelch, 18 emphasized, 19 compressed, 20 compressed_emphasized
 * 21 dsp_commands, 22 dsp_commands_samples, 23 adpcm_g721, 24 adpcm_g722, 25 adpcm_g723,
 * 26 adpcm_g723_5, 27 alaw_8, 28 aes, 29 delat_mulaw_8 {internal Snd-secret format: 30=linear_32_little_endian}
 *
 */

static void read_next_header (int chan)
{
  int maybe_bicsf;
  type_specifier = get_uninterpreted_int((unsigned char *)hdrbuf);
  data_location = get_big_endian_int((unsigned char *)(hdrbuf+4));
  data_size = get_big_endian_int((unsigned char *)(hdrbuf+8));
  /* can be bogus -- fixup if possible */
  true_file_length = lseek(chan,0L,SEEK_END);
  if (data_size <= 24) data_size = (true_file_length-data_location);
  original_sound_format = get_big_endian_int((unsigned char *)(hdrbuf+12));
  switch (original_sound_format) 
    { /* defined in /usr/include/sound/soundstruct.h */
      /* see headers.lisp for a table of all known format names */
    case 1: sound_format = snd_8_mulaw; break;
    case 2: sound_format = snd_8_linear; break;
    case 3: sound_format = snd_16_linear; break;
    case 4: sound_format = snd_24_linear; break;
    case 5: sound_format = snd_32_linear; break;
    case 6: sound_format = snd_32_float; break;
    case 7: sound_format = snd_64_double; break;
    case 18: sound_format = snd_16_linear; break; 
      /* "emphasized": Xavier Serra's de-emphasis filter: y(n) = x(n) + .9 y(n-1) */
    case 27: sound_format = snd_8_alaw; break;
    case 30: sound_format = snd_32_linear_little_endian; break; /* this is for Snd's internal benefit -- it is not used elsewhere */
    default: sound_format = snd_unsupported; break;
    }
  srate = get_big_endian_int((unsigned char *)(hdrbuf+16));
  chans = get_big_endian_int((unsigned char *)(hdrbuf+20));
  comment_start = 24;
  comment_end = data_location - 1;
  if (comment_end < comment_start) comment_end = comment_start;
  if (match_four_chars((unsigned char *)(hdrbuf+24),I_AFsp)) header_distributed = 1; else header_distributed = 0;
  maybe_bicsf = get_big_endian_int((unsigned char *)(hdrbuf+28));
  if (maybe_bicsf == 107364) read_bicsf_header(chan);
  data_size = c_snd_samples(sound_format,data_size);
}

void write_next_header (int chan, int srate, int chans, int loc, int siz, int format, char *comment, int len)
{
  int i,j;
  char *str;
  write_four_chars((unsigned char *)hdrbuf,I_DSND); /* ".snd" */
  i = len/4;
  j = 24 + (4 * (i+1));
  if (loc < j) loc = j;
  set_big_endian_int((unsigned char *)(hdrbuf+4),loc);
  set_big_endian_int((unsigned char *)(hdrbuf+8),siz);
  switch (format)
    {
    case snd_8_mulaw: set_big_endian_int((unsigned char *)(hdrbuf+12),1); break;
    case snd_8_linear: set_big_endian_int((unsigned char *)(hdrbuf+12),2); break;
    case snd_16_linear: set_big_endian_int((unsigned char *)(hdrbuf+12),3); break;
    case snd_24_linear: set_big_endian_int((unsigned char *)(hdrbuf+12),4); break;
    case snd_32_linear: set_big_endian_int((unsigned char *)(hdrbuf+12),5); break;
    case snd_32_float: set_big_endian_int((unsigned char *)(hdrbuf+12),6); break;
    case snd_64_double: set_big_endian_int((unsigned char *)(hdrbuf+12),7); break;
    case snd_32_linear_little_endian: set_big_endian_int((unsigned char *)(hdrbuf+12),30); break; /* see above */
    case snd_8_alaw: set_big_endian_int((unsigned char *)(hdrbuf+12),27); break;
    default: 
      str=(char *)CALLOC(256,sizeof(char));
      sprintf(str,"NeXT/Sun unsupported sound data format type: %d\n",format); 
      clm_printf(str);
      FREE(str);
      break;
    }
  set_big_endian_int((unsigned char *)(hdrbuf+16),srate);
  set_big_endian_int((unsigned char *)(hdrbuf+20),chans);
  write(chan,hdrbuf,24);
  j = 0;
  for (i=0;i<len;i++) 
    {
      hdrbuf[j]=comment[i];
      j++;
      if (j == HDRBUFSIZ) 
	{
	  write(chan,hdrbuf,HDRBUFSIZ);
	  j = 0;
	}
    }
  for (i=0;i<(loc-(len+24));i++) /* now fill left over bytes with nulls */
    {
      hdrbuf[j]=0;
      j++;
      if (j == HDRBUFSIZ) 
	{
	  write(chan,hdrbuf,HDRBUFSIZ);
	  j = 0;
	}
    }
  if (j != 0) write(chan,hdrbuf,j);
  data_location = loc;
}

static void update_next_header (int chan, int siz)
{
  lseek(chan,8L,SEEK_SET);
  set_big_endian_int((unsigned char *)(hdrbuf+0),siz);
  write(chan,hdrbuf,4);
}

static void update_next_header_comment (int chan, int loc, char *comment, int len)
{
  int i,j;
  lseek(chan,(long)(loc-4),SEEK_SET);
  read(chan,hdrbuf,4);
  lseek(chan,(long)(loc-4),SEEK_SET);
  for (j=0;j<4;j++) if (hdrbuf[j]==0) hdrbuf[j] = 32;
  j = 4;
  for (i=0;i<len;i++) 
    {
      hdrbuf[j]=comment[i];
      j++;
      if (j == HDRBUFSIZ) 
	{
	  write(chan,hdrbuf,HDRBUFSIZ); 
	  j = 0;
	}
    }
  if (j != 0) write(chan,hdrbuf,j);
}



/* ------------------------------------ AIFF ------------------------------------ 
 *
 *  0: "FORM"
 *  4: size (bytes)
 *  8: "AIFF" or "AIFC" -- the latter includes compressed formats (list extended for 8.5 Sound.h)
 *
 *  Thereafter the file is organized into "chunks", each chunk being 
 *  a 4-byte identifer followed by an int (4-bytes) giving the chunk size
 *  not including the 8-byte header.  AIFF data is signed.  If the chunk
 *  size is odd, an extra (unaccounted-for) null byte is added at the end.
 *
 *  The chunks we want are "COMM", "SSND", and "APPL".
 *
 * COMM: 0: chans
 *       2: frames
 *       6: bits per sample
 *       8: srate as 80-bit IEEE float
 *  then if AIFC (not AIFF), 4 bytes giving compression id ("NONE"=not compressed)
 *    followed by Pascal string giving long name of compression type
 *
 * SSND: 0: data location (offset within SSND chunk)
 *
 * Other chunks include:  ANNO: a comment, INST: loop control, MARK: marker, MIDI: midi,
 *                        COMT: comment (max 65536 chars), NAME: sound name, AUTH: author's name
 *                        (c), AESD: recording data, APPL: application specific stuff
 *    "MARK" size short-#marks {marks} -- latter are short-ID long-position pstring-name.
 *    "INST" size chars[baseNote detune lowNote highNote lowVelocity HighVelocity] short-gain loops[sustain release]
 *      loop: short-playMode marker-begin marker-end (signed?) shorts)
 *         playMode: 0 no loop, 1 forward loop, 2 forward/backward loop
 *      chars are MIDI data (detune is in cents)
 *    "MIDI" size MIDI-data...
 *    "AESD" size AES Channel Status Data (24 bytes as specified by AES)
 *      see "AES: Guidelines for the use of the AES3 interface"
 *      byte 0: bit 0: 0=consumer, 1=pro
 *              bit 1: 0=audio, 1=non-audio
 *              bits 2:4: emphasis: 0:none, 4:none, 6:CD, 7:CCITT J17
 *              bits 6:7: srate: 00=48KHz, 01=48, 10=44.1, 11=32
 *      byte 1: bits 0:3: chans: 2:mono, else stereo
 *      byte 2 for word size stuff (always ends up 16-bit): bits 3-5=sample length where 4=16-bit
 *      byte 3: multi-channels modes, 4: AES sync ref, 5:unused, 6-9:ASCII source ID, 10-13:ASCII destination ID
 *      byte 14-17:local sample addr, 18-21:time of day addr, then CRC checks
 *    "APPL" size signature data
 *    "COMT" size short-#comments {comments} -- the latter are long-time marker short-text-length char-text
 *       time is in seconds since 1-Jan-1904
 *    "NAME"/"AUTH"/"(c) "/"ANNO" size char-name
 *    "FVER" size(4) AIFC-format-version -- currently always 0xA2805140
 *    "SAXL" -- a desperate kludge to get around Apple's own compression schemes!
 *
 * always big-endian
 * There was also (briefly) an AIFS file, now deprecated.
 */

/* ieee-80 conversions -- design by committee! */
/* this code taken from CSound sources -- apparently originally written by Malcolm Slaney at Apple */

#define ULPOW2TO31	((unsigned int)0x80000000)
#define DPOW2TO31	((double)2147483648.0)	/* 2^31 */

static double myUlongToDouble(unsigned int ul)
{
  double val;
  if(ul & ULPOW2TO31) val = DPOW2TO31 + (ul & (~ULPOW2TO31));
  else val = ul;
  return val;
}

static unsigned int myDoubleToUlong(double val)
{
  unsigned int ul;
  if(val < DPOW2TO31) ul = (unsigned int)val;
  else ul = ULPOW2TO31 | (unsigned int)(val-DPOW2TO31);
  return ul;
}

static double ieee_80_to_double(unsigned char *p)
{
  unsigned char sign;
  short exp = 0;
  unsigned int mant1 = 0;
  unsigned int mant0 = 0;
  double val;
  exp = *p++;  exp <<= 8;  exp |= *p++;  sign = (exp & 0x8000) ? 1 : 0;  exp &= 0x7FFF;
  mant1 = *p++;  mant1 <<= 8;  mant1 |= *p++;  mant1 <<= 8;  mant1 |= *p++;  mant1 <<= 8;  mant1 |= *p++;
  mant0 = *p++;  mant0 <<= 8;  mant0 |= *p++;  mant0 <<= 8;  mant0 |= *p++;  mant0 <<= 8;  mant0 |= *p++;
  if(mant1 == 0 && mant0 == 0 && exp == 0 && sign == 0)
    return 0.0;
  else
    {
      val = myUlongToDouble(mant0) * pow(2.0,-63.0);
      val += myUlongToDouble(mant1) * pow(2.0,-31.0);
      val *= pow(2.0,((double) exp) - 16383.0);
      return sign ? -val : val;
    }
}

static void double_to_ieee_80(double val, unsigned char *p)
{
  unsigned char sign = 0;
  short exp = 0;
  unsigned int mant1 = 0;
  unsigned int mant0 = 0;
  if(val < 0.0)	{  sign = 1;  val = -val; }
  if(val != 0.0)	/* val identically zero -> all elements zero */
    {
      exp = (short)(log(val)/log(2.0) + 16383.0);
      val *= pow(2.0, 31.0+16383.0-(double)exp);
      mant1 = myDoubleToUlong(val);
      val -= myUlongToDouble(mant1);
      val *= pow(2.0, 32.0);
      mant0 = myDoubleToUlong(val);
    }
  *p++ = ((sign<<7)|(exp>>8));  *p++ = 0xFF & exp;  
  *p++ = 0xFF & (mant1>>24);  *p++ = 0xFF & (mant1>>16);  *p++ = 0xFF & (mant1>> 8);  *p++ = 0xFF & (mant1);
  *p++ = 0xFF & (mant0>>24);  *p++ = 0xFF & (mant0>>16);  *p++ = 0xFF & (mant0>> 8);  *p++ = 0xFF & (mant0);
}


static int update_form_size, update_frames_location, update_ssnd_location;

static int seek_and_read(int chan, unsigned char *buf, int offset, int nbytes)
{
  if (offset < 0) return(-1);
  lseek(chan, offset, SEEK_SET);
#ifndef MACOS
  return(read(chan,buf,nbytes));
#else
  return(read(chan,(char *)buf,nbytes));
#endif
}

static int read_aiff_marker(int m, unsigned char *buf)
{
  int psize;
  marker_ids[m] = get_big_endian_short((unsigned char *)buf);
  marker_positions[m] = get_big_endian_int((unsigned char *)(buf+2));
  psize = (int)buf[6] + 1; 
  if (psize & 1) psize++; 
  return(psize+6);
}

static void read_aiff_header (int chan, int overall_offset)
{
  /* we know we have checked for FORM xxxx AIFF|AIFC when we arrive here */
  /* as far as I can tell, the COMM block has the header data we seek, and the SSND block has the sound data */
  /* everything else will be ignored -- apparently we can depend on seeing a "chunk" name, then size */
  int chunksize,offset,frames,chunkloc,happy,i,j,num_marks,m,moff,msize;
  type_specifier = get_uninterpreted_int((unsigned char *)(hdrbuf+8+overall_offset));
  update_ssnd_location = 0;
  chunkloc = 12 + overall_offset;
  offset = 0;
  for (i=0;i<AUX_COMMENTS;i++) aux_comment_start[i] = 0;
  sound_format = snd_16_linear;
  header_distributed = 1;
  srate = 0;
  chans = 0;
  happy = 1;
  data_size = 0;
  if (loop_modes)
    {
      loop_modes[0] = 0;
      loop_modes[1] = 0;
    }
  true_file_length = lseek(chan,0L,SEEK_END);
  update_form_size = get_big_endian_int((unsigned char *)(hdrbuf+4+overall_offset)); /* should be file-size-8 unless there are multiple forms */
  while (happy)
    {
      offset += chunkloc;
      if (seek_and_read(chan,(unsigned char *)hdrbuf,offset,32) <= 0)
	{
	  clm_printf("error reading header");
	  return;
	}
      chunksize = get_big_endian_int((unsigned char *)(hdrbuf+4));
      if (match_four_chars((unsigned char *)hdrbuf,I_COMM))
	{
	  chans = get_big_endian_short((unsigned char *)(hdrbuf+8));
	  frames = get_big_endian_int((unsigned char *)(hdrbuf+10));
	  update_frames_location = 10+offset;
	  original_sound_format = get_big_endian_short((unsigned char *)(hdrbuf+14));
	  if ((original_sound_format%8) != 0) 
	    {
	      /* weird sizes are legal --
	       * these samples are left-justified (and zero padded on the right), so
	       * we can handle any bit size by rounding up to the nearest byte.
	       */
	      original_sound_format=8*(1+(original_sound_format>>3));
	    }
	  if (original_sound_format == 8) sound_format = snd_8_linear;
	  else if (original_sound_format == 16) sound_format = snd_16_linear;
	  else if (original_sound_format == 24) sound_format = snd_24_linear;
	  else if (original_sound_format == 32) sound_format = snd_32_linear;
	  else sound_format = snd_unsupported;
	  srate = (int)ieee_80_to_double((unsigned char *)(hdrbuf+16));
	  /* if AIFC, compression type over-rides (possibly bogus) original_sound_format */
	  if (type_specifier == get_uninterpreted_int((unsigned char *)I_AIFC))
	    {
	      /* some aifc files assume the compression field is a new and very weird chunk!! -- surely a bug? */
	      /* AIFF spec says COMM size is always 18, but this is amended in the newer AIFC spec */
	      if (chunksize == 18) chunksize += (5+((int)hdrbuf[30])); /* 5=chunk header length in this case */
	      if ((!(match_four_chars((unsigned char *)(hdrbuf+26),I_NONE))) &&
		  (!(match_four_chars((unsigned char *)(hdrbuf+26),I_twos))))
		{
		  original_sound_format = get_uninterpreted_int((unsigned char *)(hdrbuf+26));
		  if ((match_four_chars((unsigned char *)(hdrbuf+26),I_ALAW)) || 
		      (match_four_chars((unsigned char *)(hdrbuf+26),I_alaw)))
		    sound_format = snd_8_alaw;
		  else 
		    {
		      if ((match_four_chars((unsigned char *)(hdrbuf+26),I_ULAW)) ||
			  (match_four_chars((unsigned char *)(hdrbuf+26),I_ulaw)))
			sound_format = snd_8_mulaw;
		      else 
			{
			  /* taken from Sound.h in OS 8.5 -- just guessing... */
			  if (match_four_chars((unsigned char *)(hdrbuf+26),I_sowt))
			    {
			      if (sound_format == snd_16_linear) sound_format = snd_16_linear_little_endian;
			      else if (sound_format == snd_24_linear) sound_format = snd_24_linear_little_endian;
			      else if (sound_format == snd_32_linear) sound_format = snd_32_linear_little_endian;
			    }
			  else
			    {
			      if (match_four_chars((unsigned char *)(hdrbuf+26),I_raw_))
				{
				  if (sound_format == snd_8_linear) sound_format = snd_8_unsigned;
				  else if (sound_format == snd_16_linear) sound_format = snd_16_unsigned;
				}
			      else
				{
				  if (match_four_chars((unsigned char *)(hdrbuf+26),I_fl32))
				    sound_format = snd_32_float;
				  else
				    {
				      if (match_four_chars((unsigned char *)(hdrbuf+26),I_fl64))
					sound_format = snd_64_double;
				      else
					{
					  if (match_four_chars((unsigned char *)(hdrbuf+26),I_ima4))
					    {
					      block_align = 34;
					      original_sound_format = AIFF_IMA_ADPCM;
					    }
					  sound_format = snd_unsupported;
					}
				    }
				}
			    }
			}
		    }
		}
	    }
	  data_size = (frames*c_snd_datum_size(sound_format)*chans);
	}
      else
	{
	  if (match_four_chars((unsigned char *)hdrbuf,I_SSND))
	    {
	      update_ssnd_location = offset+4;
	      data_location = get_big_endian_int((unsigned char *)(hdrbuf+8)) + offset + 16; /* Baroque! */
	      /* offset is where the hdrbuf is positioned in the file, the sound data offset itself is at loc+8 and the */
	      /* 0-based location of the sound data is at the end of the chunk = 16 (8=header+4=offset+4=blocksize) */
	      /* the next int can be the block size if the data is block-aligned */
	      /* only one SSND per AIFF is allowed */
	    }
	  else
	    {
	      if ((match_four_chars((unsigned char *)hdrbuf,I_ANNO)) || (match_four_chars((unsigned char *)hdrbuf,I_COMT)))
		{
		  j=0;
		  for (i=0;i<AUX_COMMENTS;i++) if (aux_comment_start[i] == 0) {j=i; break;}
		  if (j >= AUX_COMMENTS) {clm_printf("ran out of auxiliary comment space"); j=0;}
		  aux_comment_start[j] = offset+8;
		  if (match_four_chars((unsigned char *)hdrbuf,I_COMT)) aux_comment_start[j] += 8; /* skip time stamp and markerId (not ID, I assume!) */
		  aux_comment_end[j] = offset+7+chunksize;
		}
	      else
		{
		  if (match_four_chars((unsigned char *)hdrbuf,I_APPL))
		    {
		      if (match_four_chars((unsigned char *)(hdrbuf+8),I_CLM_))
			{
			  /* my own chunk has the arbitrary length comment I use (actually the ASCII    */
			  /* representation of a lisp program evaluated in the CLM package) to handle mix et al. */
			  /* It is nothing more than the actual string -- remember to pad to even length here. */
			  comment_start = offset + 12;
			  comment_end = comment_start + chunksize - 5;
			}
		    }
		  else
		    {
		      if (match_four_chars((unsigned char *)hdrbuf,I_INST))
			{
			  base_note = hdrbuf[8];
			  base_detune = hdrbuf[9];
			  loop_modes[0] = get_big_endian_short((unsigned char *)(hdrbuf+16));
			  loop_starts[0] = get_big_endian_short((unsigned char *)(hdrbuf+18));
			  loop_ends[0] = get_big_endian_short((unsigned char *)(hdrbuf+20));
			  loop_modes[1] = get_big_endian_short((unsigned char *)(hdrbuf+22));
			  loop_starts[1] = get_big_endian_short((unsigned char *)(hdrbuf+24));
			  loop_ends[1] = get_big_endian_short((unsigned char *)(hdrbuf+26));
			  /* these are mark numbers */
			}
		      else
			{
			  if (match_four_chars((unsigned char *)hdrbuf,I_MARK))
			    {
			      /* unsigned short #marks, each mark: id pos name (pstring damn it) */
			      num_marks = get_big_endian_unsigned_short((unsigned char *)(hdrbuf+8));
			      if (num_marks > markers)
				{
				  if (markers > 0) {if (marker_ids) FREE(marker_ids); if (marker_positions) FREE(marker_positions);}
				  markers = num_marks;
				  marker_ids = (int *)CALLOC(markers,sizeof(int));
				  marker_positions = (int *)CALLOC(markers,sizeof(int));
				}
			      moff = 10;
			      for (m=0;m<num_marks;m++)
				{
				  if (seek_and_read(chan,(unsigned char *)hdrbuf,offset+moff,8) > 0)
				    {
				      msize = read_aiff_marker(m,(unsigned char *)hdrbuf);
				      moff += msize;
				    }
				}
			    }
			}
		    }
		}
	    }
	}
      chunkloc = (8+chunksize);
      if (chunksize&1) chunkloc++; /* extra null appended to odd-length chunks */
      if ((offset+chunkloc) >= update_form_size) happy=0;
    }
  if (true_file_length < data_size) data_size = true_file_length - data_location;
  data_size = c_snd_samples(sound_format,data_size);
}

static int aifc_header = 1;
void set_aifc_header(int val) {aifc_header = val;}

static void write_aiff_header (int chan, int srate, int chans, int siz, int format, char *comment, int len)
{
  /* we write the simplest possible AIFC header: AIFC | COMM | APPL-CLM_ if needed | SSND eof. */
  /* the assumption being that we're going to be appending sound data once the header is out   */
  /* INST and MARK chunks added Jul-95 for various programs that expect them (MixView).        */
  int i,j,lenhdr,curend,extra; /* set aifc to 0 to get old-style AIFF header */
  char *str;
  lenhdr=0;
  extra=0;
  curend=0;
  if (len != 0) 
    {
      lenhdr = 12;
      if ((len % 4) != 0)
	extra = (4 - (len % 4));
    }
  write_four_chars((unsigned char *)hdrbuf,I_FORM);
  if (aifc_header) 
    set_big_endian_int((unsigned char *)(hdrbuf+4),len+30+16+38+siz+lenhdr+extra+12+10);
  else set_big_endian_int((unsigned char *)(hdrbuf+4),len+30+16+38+siz+lenhdr+extra);
  /* 
   * comment length + 4 for AIFF 18+8 for I_COMM info + 16 for I_SSND info + 38 for INST and MARK +
   * siz for data + 12 for comment header if any + padding == total size - 8 (i.e. FORM header).   
   * INST+MARK (38) added 3-Jul-95 for Notam software compatibility 
   */
  if (aifc_header) 
    {
      write_four_chars((unsigned char *)(hdrbuf+8),I_AIFC); 
      write(chan,hdrbuf,12);
      curend=12;
      write_four_chars((unsigned char *)hdrbuf,I_FVER);
      set_big_endian_int((unsigned char *)(hdrbuf+4),4);
      set_big_endian_int((unsigned char *)(hdrbuf+8),0xA2805140);
    }
  else write_four_chars((unsigned char *)(hdrbuf+8),I_AIFF);
  write_four_chars((unsigned char *)(hdrbuf+12),I_COMM);
  if (aifc_header) set_big_endian_int((unsigned char *)(hdrbuf+16),18+10); else set_big_endian_int((unsigned char *)(hdrbuf+16),18);
  set_big_endian_short((unsigned char *)(hdrbuf+20),(short)chans);
  set_big_endian_int((unsigned char *)(hdrbuf+22),siz / (chans*c_snd_datum_size(format)));
  switch (format)
    {
    case snd_16_linear: case snd_16_linear_little_endian: case snd_16_unsigned: 
      set_big_endian_short((unsigned char *)(hdrbuf+26),16); 
      break;
    case snd_24_linear: case snd_24_linear_little_endian: 
      set_big_endian_short((unsigned char *)(hdrbuf+26),24); 
      break;
    case snd_32_linear: case snd_32_linear_little_endian: case snd_32_float: 
      set_big_endian_short((unsigned char *)(hdrbuf+26),32); 
      break;
    case snd_64_double: 
      set_big_endian_short((unsigned char *)(hdrbuf+26),64); 
      break;
    case snd_8_linear: case snd_8_unsigned: 
      set_big_endian_short((unsigned char *)(hdrbuf+26),8); 
      break;
    case snd_8_mulaw: 
      set_big_endian_short((unsigned char *)(hdrbuf+26),8); 
      break;
    case snd_8_alaw: 
      set_big_endian_short((unsigned char *)(hdrbuf+26),8); 
      break;
    default: 
      str=(char *)CALLOC(256,sizeof(char));
      sprintf(str,"AIFF unsupported sound data format type: %d\n",format);
      clm_printf(str);
      FREE(str);
      break;
    }
  double_to_ieee_80((double)srate,(unsigned char *)(hdrbuf+28));
  if (aifc_header)
    {
      switch (format)
	{
	case snd_16_linear: case snd_24_linear: case snd_32_linear: case snd_8_linear: str = (char *)I_NONE; break;
	case snd_16_linear_little_endian: case snd_24_linear_little_endian: case snd_32_linear_little_endian: str = (char *)I_sowt; break;
	case snd_32_float: str = (char *)I_fl32; break;
	case snd_64_double: str = (char *)I_fl64; break;
	case snd_8_unsigned: case snd_16_unsigned: str = (char *)I_raw_; break;
	case snd_8_mulaw: str = (char *)I_ulaw; break;
	case snd_8_alaw: str = (char *)I_alaw; break;
	default: str = (char *)I_NONE; break;
	}
      write_four_chars((unsigned char *)(hdrbuf+38),(const unsigned char *)str);
      (*(unsigned char *)(hdrbuf+42)) = 4; /* final pad null not accounted-for */
      write_four_chars((unsigned char *)(hdrbuf+43),(const unsigned char *)str);
      (*(unsigned char *)(hdrbuf+47)) = 0;
      i=48;
    }
  else i = 38;
  if (len != 0)
    {
      if (aifc_header)
	{
	  write_four_chars((unsigned char *)(hdrbuf+48),I_APPL);
	  set_big_endian_int((unsigned char *)(hdrbuf+52),len+4+extra);
	  write_four_chars((unsigned char *)(hdrbuf+56),I_CLM_);
	  i = 60;
	}
      else
	{
	  write_four_chars((unsigned char *)(hdrbuf+38),I_APPL);
	  set_big_endian_int((unsigned char *)(hdrbuf+42),len+4+extra);
	  write_four_chars((unsigned char *)(hdrbuf+46),I_CLM_);
	  i = 50;
	}
      for (j=0;j<len;j++)
	{
	  if (i == HDRBUFSIZ)
	    {
	      curend += HDRBUFSIZ;
	      write(chan,hdrbuf,HDRBUFSIZ);
	      i=0;
	    }
	  hdrbuf[i]=comment[j];
	  i++;
	}
      if (extra != 0)
	{
	  if ((i+extra) > HDRBUFSIZ)
	    {
	      curend += i;
	      write(chan,hdrbuf,i);
	      i=0;
	    }
	  for (j=0;j<extra;j++)
	    {
	      hdrbuf[i] = 0;
	      i++;
	    }
	}
    }
  curend += i;
  write(chan,hdrbuf,i);
  write_four_chars((unsigned char *)hdrbuf,I_MARK);   /* SoundHack includes a blank MARK chunk for some reason */
  set_big_endian_int((unsigned char *)(hdrbuf+4),2);
  set_big_endian_short((unsigned char *)(hdrbuf+8),0);
  write_four_chars((unsigned char *)(hdrbuf+10),I_INST);
  set_big_endian_int((unsigned char *)(hdrbuf+14),20);
  set_big_endian_int((unsigned char *)(hdrbuf+18),0x3c00007f); /* base-note=middle C, detune=0, lownote=0, highnote=0x7f */
  set_big_endian_int((unsigned char *)(hdrbuf+22),0x017f0000); /* lowvelocity=1, highvelocity=0x7f, gain=0 */
  set_big_endian_int((unsigned char *)(hdrbuf+26),0);          /* no loops */
  set_big_endian_int((unsigned char *)(hdrbuf+30),0); 
  set_big_endian_int((unsigned char *)(hdrbuf+34),0); 
  write_four_chars((unsigned char *)(hdrbuf+38),I_SSND);
  set_big_endian_int((unsigned char *)(hdrbuf+38+4),siz+8);
  set_big_endian_int((unsigned char *)(hdrbuf+38+8),0);                        /* "offset" */
  set_big_endian_int((unsigned char *)(hdrbuf+38+12),0);                       /* "blocksize " */
  data_location = 38+16+curend;
  write(chan,hdrbuf,38+16);
}


static void update_aiff_header (int chan, int siz)
{
  /* we apparently have to make sure the form size and the data size are correct 
   * assumed here that we'll only be updating our own AIFF files 
   * There are 3 such locations -- the 2nd word of the file which is the overall form size, 
   * the frames variable in the COMM chunk, and the chunk-size variable in the SSND chunk 
   * an unexpected hassle for CLM is that we can open/close the output file many times if running mix,
   * so we have to update the various size fields taking into account the old size 
   */
  read(chan,hdrbuf,INITIAL_READ_SIZE);
  read_aiff_header(chan,0);
  lseek(chan,4L,SEEK_SET);
  set_big_endian_int((unsigned char *)hdrbuf,siz+update_form_size-c_snd_bytes(sound_format,data_size));
  /* cancel old data_size from previous possible write */
  write(chan,hdrbuf,4);
  lseek(chan,update_frames_location,SEEK_SET);
  set_big_endian_int((unsigned char *)hdrbuf,siz/(chans*c_snd_datum_size(sound_format)));
  write(chan,hdrbuf,4);
  lseek(chan,update_ssnd_location,SEEK_SET);
  set_big_endian_int((unsigned char *)hdrbuf,siz+8);
  write(chan,hdrbuf,4);
}

static void update_aiff_header_comment (int chan, char *comment, int len)
{
  /* save-stats in CLM appends a comment after the sound data has been written */
  int i,j,true_len,old_len;
  if (len&1) true_len = len+1; else true_len=len;
  lseek(chan,0L,SEEK_END);
  write_four_chars((unsigned char *)hdrbuf,I_ANNO);
  set_big_endian_int((unsigned char *)(hdrbuf+4),len);
  for (i=0,j=8;i<len;i++,j++) hdrbuf[j]=comment[i];
  write(chan,hdrbuf,8+true_len);
  lseek(chan,4L,SEEK_SET);
  read(chan,hdrbuf,4);
  old_len = get_big_endian_int((unsigned char *)hdrbuf);
  set_big_endian_int((unsigned char *)hdrbuf,old_len+true_len+8);
  lseek(chan,4L,SEEK_SET);
  write(chan,hdrbuf,4);
}



/* ------------------------------------ RIFF (wave) ------------------------------------
 *
 * see ftp.microsoft.com:/SoftLib/MSLFILES/MDRK.EXE (also MMSYSTEM.H and MMREG.H)
 *     ftp://ftp.isi.edu/in-notes/rfc2361.txt
 *
 *   0: "RIFF" (little-endian) or "RIFX" (big-endian)
 *   4: size
 *   8: "WAVE"  ("RMID" = midi data, others are AVI, CPPO, ACON etc)
 *       AVI chunk can include audio data
 *  
 *   rest very similar to AIFF (odd-sized chunks are padded)
 *
 * fmt  0: format code (see below)
 *      2: chans
 *      4: srate (long)
 *      8: average rate "for buffer estimation"
 *     12: alignment "block size"
 *     14: data size (bits per sample) (PCM only)
 *     16: count (bytes) of extra info in the header (i.e. trailing info added to this basic header)
 *     20: samples per block (short) in dvi_adpcm
 *  
 * formats are: 0: unknown, 1: PCM, 2: ADPCM, 3: IEEE float, 4: VSELP, 5: IBM_CVSD, 6: alaw, 7: mulaw
 *              0x10: OKI_ADPCM, 0x11: DVI_ADPCM, 0x12: MediaSpace_ADPCM,
 *              0x13: Sierra_ADPCM, 0x14: G723_ADPCM, 0x15: DIGISTD, 0x16: DIGIFIX, 0x17: Dialogic ADPCM,
 *              0x18: Mediavision ADPCM, 0x19: HP cu codec, 
 *              0x20: Yamaha_ADPCM, 0x21: SONARC, 0x22: DSPGroup_TrueSpeech
 *              0x23: EchoSC1, 0x24: AudioFile_AF36, 0x25: APTX, 0x26: AudioFile_AF10
 *              0x27: prosody 1612, 0x28: lrc,
 *              0x30: Dolby_Ac2, 0x31: GSM610, 0x32: MSN audio codec, 0x33: Antext_ADPCM, 0x34: Control_res_vqlpc,
 *              0x35: DIGIREAL, 0x36: DIGIADPCM, 0x37: Control_res_cr10, 0x38: NMS_VBXADPCM, 0x39:Roland rdac,
 *              0x3a: echo sc3, 0x3b: Rockwell adpcm, 0x3c: Rockwell digitalk codec, 0x3d: Xebec,
 *              0x40: G721_ADPCM, 0x41: G728 CELP, 0x42: MS G723, 0x50: MPEG, 
 *              0x52: RT24, 0x53: PAC, 0x55: Mpeg layer 3, 0x59: Lucent G723, 0x60: Cirrus,
 *              0x61: ESS Tech pcm, 0x62: voxware (obsolete), 0x63: canopus atrac,
 *              0x64: G726, 0x65: G722, 0x66: DSAT, 0x67: DSAT display,
 *              0x69: voxware (obsolete), 0x70: voxware ac8 (obsolete), 0x71: voxware ac10 (obsolete), 
 *              0x72: voxware ac16 (obsolete), 0x73: voxware ac20 (obsolete), 0x74: voxware rt24, 
 *              0x75: voxware rt29, 0x76: voxware rt29hw (obsolete), 0x77: voxware vr12 (obsolete),
 *              0x78: voxware vr18 (obsolete), 0x79: voxware tq40 (obsolete), 
 *              0x80: softsound, 0x81: voxware tq60 (obsolete), 0x82: MS RT24, 0x83: G729A,
 *              0x84: MVI_MVI2, 0x85: DF G726, 0x86: DF GSM610, 0x88: isaudio, 0x89: onlive,
 *              0x91: sbc24, 0x92: dolby ac3 spdif, 0x97: zyxel adpcm, 0x98: philips lpcbb,
 *              0x99: packed, 0x100: rhetorex adpcm, 
 *              0x101: Irat, 0x102: IBM_alaw?, 0x103: IBM_ADPCM?, 
 *              0x111: vivo G723, 0x112: vivo siren, 0x123: digital g273
 *              0x200: Creative_ADPCM, 0x202: Creative fastspeech 8, 0x203: Creative fastspeech 10, 
 *              0x220: quarterdeck, 0x300: FM_TOWNS_SND, 0x400: BTV digital, 0x680: VME vmpcm,
 *              0x1000: OLIGSM, 0x1001: OLIADPCM, 0x1002: OLICELP, 0x1003: OLISBC, 0x1004: OLIOPR
 *              0x1100: LH codec, 0x1400: Norris, 0x1401: isaudio, 0x1500: Soundspace musicompression, 0x2000: DVM
 * (see http://www.microsoft.com/asf/resources/draft-ietf-fleischman-codec-subtree-00.txt)
 *
 * RIFF and LIST chunks have nested chunks.  Registered chunk names include:
 *   LIST with subchunks, one of which can be:
 *     INFO itself containing:
 *       IARL: archival location, IART: artist, ICMS: commissioned, ICMT: comment, ICOP: copyright, ICRD: creation date,
 *       ICRP: uh...cropped, IDIM: dimensions, IDPI: dpi, IENG: engineer, IGNR: genre, IKEY: keywords, ILGT: lightness,
 *       IMED: medium, INAM: name, IPLT: palette, IPRD: product, ISBJ: subject, ISFT: software, ISHP: sharpness,
 *       ISRC: source, ISRF: source form, ITCH: technician, ISMP: SMPTE time code, IDIT: digitization time
 *
 * data chunk has the samples
 * other (currently ignored) chunks are wavl = waveform data, fact, cues of some sort, slnt = silence,
 *     plst = playlist, adtl = associated data list, labl = cue label, note = cue comments,
 *     ltxt = text associated with data segment (cue), file, DISP = displayable object,
 *     JUNK = outdated info, PAD = padding, etc
 * fact chunk generally has number of samples (used in compressed files)
 */

static int wave_to_sndlib_format(int osf, int bps, int little)
{
  switch (osf)
    {
    case 1:
      switch (bps)
	{
	case 8: return(snd_8_unsigned); break;
	case 16: if (little) return(snd_16_linear_little_endian); else return(snd_16_linear); break;
	case 32: if (little) return(snd_32_linear_little_endian); else return(snd_32_linear); break;
	case 24: if (little) return(snd_24_linear_little_endian); else return(snd_24_linear); break;
	default: return(snd_8_unsigned); break;
	}
      break;
    case 3: if (little) return(snd_32_float_little_endian); else return(snd_32_float); break;
    case 6: if (bps == 8) return(snd_8_alaw); break;
    case 7: if (bps == 8) return(snd_8_mulaw); break;
      /* IBM mulaw follows G711 specs like other versions (this info direct from IBM) */
    case 0x101: return(snd_8_mulaw); break;
    case 0x102: return(snd_8_alaw); break;
    }
  return(snd_unsupported);
}

static void read_riff_header (int chan)
{
  /* we know we have checked for RIFF xxxx WAVE when we arrive here */
  int chunksize,offset,chunkloc,happy,little;
  little = 1;
  if (match_four_chars((unsigned char *)hdrbuf,I_RIFX)) little=0; /* big-endian data in this case, but I've never seen one */
  type_specifier = get_uninterpreted_int((unsigned char *)(hdrbuf+8));
  chunkloc = 12;
  offset = 0;
  header_distributed = 1;
  sound_format = snd_unsupported;
  srate = 0;
  chans = 0;
  happy = 1;
  data_size = 0;
  fact_samples = 0;
  bits_per_sample = 0;
  true_file_length = lseek(chan,0L,SEEK_END);
  update_form_size = big_or_little_endian_int((unsigned char *)(hdrbuf+4),little);
  while (happy)
    {
      offset += chunkloc;
      if (seek_and_read(chan,(unsigned char *)hdrbuf,offset,32) <= 0)
	{
	  clm_printf("error reading header");
	  return;
	}
      chunksize = big_or_little_endian_int((unsigned char *)(hdrbuf+4),little);
      if (match_four_chars((unsigned char *)hdrbuf,I_fmt_))
	{
	  /*
	   * 8:  short format code        --1=PCM for example
	   * 10: short chans              --1
	   * 12: long rate                --48000 (0xbb80)
	   * 16: long ave rate            --65655 (0x10077)
	   * 20: short align              --2
	   * 22: short data size (bits)   --16
	   * 24: bytes of extra
	   * ... some extra data dependent on format
	   *
	   *  R I  F F  # #  # #  W A  V E  f m  t sp
	   *  5249 4646 f851 0500 5741 5645 666d 7420
	   *  e40f 0000 0100 0100 80bb 0000 0077 0100
	   *  0200 1000 0000 0000 0000 0000 0000 0000
	   *  
	   *  #x000551f8 = 348664 = size in bytes - 8
	   *  #x00000fe4 = 4068 [fmt_ chunk size?]
	   */
	  original_sound_format = big_or_little_endian_short((unsigned char *)(hdrbuf+8),little);
	  chans = big_or_little_endian_short((unsigned char *)(hdrbuf+10),little);
	  srate = big_or_little_endian_int((unsigned char *)(hdrbuf+12),little);
	  block_align = big_or_little_endian_short((unsigned char *)(hdrbuf+20),little);
	  bits_per_sample = big_or_little_endian_short((unsigned char *)(hdrbuf+22),little);
	  sound_format = wave_to_sndlib_format(original_sound_format,bits_per_sample,little);
	}
      else
	{
	  if (match_four_chars((unsigned char *)hdrbuf,I_data))
	    {
	      update_ssnd_location = offset+4;
	      data_location = offset + 8;
	      data_size = big_or_little_endian_int((unsigned char *)(hdrbuf+4),little);
	      happy = 0;
	    }
	  else
	    {
	      if (match_four_chars((unsigned char *)hdrbuf,I_fact))
		{
		  fact_samples = big_or_little_endian_int((unsigned char *)(hdrbuf+8),little);
		}
	      else
		{
		  if (match_four_chars((unsigned char *)hdrbuf,I_inst))
		    {
		      base_note = hdrbuf[8];
		      base_detune = hdrbuf[9];
		      /* rest is gain low-note high-note low-velocity high-velocity */
		    }
		  else
		    {
		      if (match_four_chars((unsigned char *)hdrbuf,I_clm_))
			{
			  comment_start = offset + 8;
			  comment_end = comment_start + chunksize - 1; /* end of comment not start of next chunk */
			}
		    }
		}
	    }
	}
      chunkloc = (8+chunksize);
      if (chunksize&1) chunkloc++; /* extra null appended to odd-length chunks */
    }
  if (true_file_length < data_size) data_size = true_file_length - data_location;
  data_size = c_snd_samples(sound_format,data_size);
}

static void write_riff_header (int chan, int srate, int chans, int siz, int format, char *comment, int len)
{
  int offset,i,j,lenhdr,extra,curend;
  char *str;
  lenhdr=0;
  extra=0;
  if (len != 0) 
    {
      lenhdr = 12;
      if ((len % 4) != 0)
	extra = (4 - (len % 4));
    }
  write_four_chars((unsigned char *)hdrbuf,I_RIFF);
  set_little_endian_int((unsigned char *)(hdrbuf+4),len+36+siz+lenhdr+extra);
  write_four_chars((unsigned char *)(hdrbuf+8),I_WAVE);
  write_four_chars((unsigned char *)(hdrbuf+12),I_fmt_);
  set_little_endian_int((unsigned char *)(hdrbuf+16),24-8);
  switch (format)
    {
    case snd_8_mulaw: 
      set_little_endian_short((unsigned char *)(hdrbuf+20),7); 
      set_little_endian_short((unsigned char *)(hdrbuf+34),8); 
      break;
    case snd_8_alaw: 
      set_little_endian_short((unsigned char *)(hdrbuf+20),6); 
      set_little_endian_short((unsigned char *)(hdrbuf+34),8); 
      break;
    case snd_8_unsigned: 
      set_little_endian_short((unsigned char *)(hdrbuf+20),1); 
      set_little_endian_short((unsigned char *)(hdrbuf+34),8); 
      break;
    case snd_16_linear_little_endian: 
      set_little_endian_short((unsigned char *)(hdrbuf+20),1); 
      set_little_endian_short((unsigned char *)(hdrbuf+34),16); 
      break;
    case snd_24_linear_little_endian: 
      set_little_endian_short((unsigned char *)(hdrbuf+20),1); 
      set_little_endian_short((unsigned char *)(hdrbuf+34),24); 
      break;
    case snd_32_linear_little_endian: 
      set_little_endian_short((unsigned char *)(hdrbuf+20),1); 
      set_little_endian_short((unsigned char *)(hdrbuf+34),32); 
      break;
    case snd_32_float_little_endian: 
      set_little_endian_short((unsigned char *)(hdrbuf+20),3); 
      set_little_endian_short((unsigned char *)(hdrbuf+34),32); 
      break;
    default: 
      str=(char *)CALLOC(256,sizeof(char));
      sprintf(str,"RIFF/Wave unsupported sound data format type: %d\n",format);
      clm_printf(str);
      FREE(str);
      break;
    }
  set_little_endian_short((unsigned char *)(hdrbuf+22),(short)chans);
  set_little_endian_int((unsigned char *)(hdrbuf+24),srate);
  set_little_endian_int((unsigned char *)(hdrbuf+28),srate * chans * c_snd_datum_size(format)); /* added chans 10-Mar-99 */
  set_little_endian_short((unsigned char *)(hdrbuf+32),(short)(chans * c_snd_datum_size(format)));

  offset = 36;
  i = 36;
  curend = 0;
  if (len != 0)
    {
      offset += len+12;
      write_four_chars((unsigned char *)(hdrbuf+36),I_clm_);
      set_little_endian_int((unsigned char *)(hdrbuf+40),len+extra);
      i = 44;
      for (j=0;j<len;j++)
	{
	  if (i == HDRBUFSIZ)
	    {
	      curend += HDRBUFSIZ;
	      write(chan,hdrbuf,HDRBUFSIZ);
	      i=0;
	    }
	  hdrbuf[i]=comment[j];
	  i++;
	}
      if (extra != 0)
	{
	  if ((i+extra) > HDRBUFSIZ)
	    {
	      curend += i;
	      write(chan,hdrbuf,i);
	      i=0;
	    }
	  for (j=0;j<extra;j++)
	    {
	      hdrbuf[i] = 0;
	      i++;
	    }
	}
    }
  curend += i;
  write(chan,hdrbuf,i);
  write_four_chars((unsigned char *)hdrbuf,I_data);
  set_little_endian_int((unsigned char *)(hdrbuf+4),siz);
  data_location = 8+curend;
  write(chan,hdrbuf,8);
}


static void update_riff_header (int chan, int siz)
{
  read(chan,hdrbuf,INITIAL_READ_SIZE);
  read_riff_header(chan);
  lseek(chan,4L,SEEK_SET);
  set_little_endian_int((unsigned char *)hdrbuf,(siz+update_form_size-c_snd_bytes(sound_format,data_size))); 
  /* see update_aiff_header for explanation */
  write(chan,hdrbuf,4);
  lseek(chan,update_ssnd_location,SEEK_SET);
  set_little_endian_int((unsigned char *)hdrbuf,siz);
  write(chan,hdrbuf,4);
}


static void update_riff_header_comment (int chan, char *comment, int len)
{
  /* save-stats in CLM appends a comment after the sound data has been written */
  int i,j,true_len,old_len;
  if (len&1) true_len = len+1; else true_len=len;
  lseek(chan,0L,SEEK_END);
  write_four_chars((unsigned char *)hdrbuf,I_INFO);
  set_little_endian_int((unsigned char *)(hdrbuf+4),len);
  for (i=0,j=8;i<len;i++,j++) hdrbuf[j]=comment[i];
  write(chan,hdrbuf,8+true_len);
  lseek(chan,4L,SEEK_SET);
  read(chan,hdrbuf,4);
  old_len = get_little_endian_int((unsigned char *)hdrbuf);
  set_little_endian_int((unsigned char *)hdrbuf,old_len+true_len+8);
  lseek(chan,4L,SEEK_SET);
  write(chan,hdrbuf,4);
}


/* ------------------------------------ AVI ------------------------------------
 * actually a video format, but it sometimes contains embedded 'wave' data
 *
 * RIFF xxx AVI 
 *   <various LISTs>
 *   LIST xxxx hdr1 LIST strl(?) strh | strf | strn etc
 *     strf is the WAVE header starting with the sound format
 *   LIST xxxx movi ##db|##wb -- wb subblocks have the audio data (these need to be collected as a single stream)
 * there are many complications that we make no effort to handle here
 *
 * described in http://www.rahul.net/jfm/avi.html
 */

static void read_avi_header (int chan)
{
  /* we know we have checked for RIFF xxxx AVI  when we arrive here */
  int chunksize,offset,chunkloc,happy,cksize,rdsize,ckoff,cktotal;
  int cksizer,ckoffr,cktotalr,bits;
  type_specifier = get_uninterpreted_int((unsigned char *)(hdrbuf+8));
  chunkloc = 12;
  offset = 0;
  header_distributed = 1;
  sound_format = snd_unsupported;
  srate = 0;
  chans = 1;
  happy = 1;
  data_size = 0;
  data_location = 0;
  true_file_length = lseek(chan,0L,SEEK_END);
  while (happy)
    {
      offset += chunkloc;
      if (seek_and_read(chan,(unsigned char *)hdrbuf,offset,32) <= 0)
	{
	  clm_printf("error reading header");
	  return;
	}
      chunksize = get_little_endian_int((unsigned char *)(hdrbuf+4));
      if (match_four_chars((unsigned char *)hdrbuf,I_LIST))
	{
	  ckoff = offset+12;
	  cktotal = 12;
	  if (match_four_chars((unsigned char *)(hdrbuf+8),I_movi))
	    {
	      while (cktotal < chunksize)
		{
		  lseek(chan,ckoff,SEEK_SET);
		  read(chan,hdrbuf,8);
		  cksize = get_little_endian_int((unsigned char *)(hdrbuf+4));
		  if ((hdrbuf[2] == 'w') && (hdrbuf[3] == 'b'))
		    {
		      data_location = ckoff;
		      if (srate != 0) happy=0;
		      break;
		    }
		  ckoff += (8+cksize);
		  cktotal += (8+cksize);
		}
	    }
	  else
	    {
	      while (cktotal < chunksize)
		{
		  lseek(chan,ckoff,SEEK_SET);
		  read(chan,hdrbuf,8);
		  cksize = get_little_endian_int((unsigned char *)(hdrbuf+4));
		  ckoff += (8+cksize);
		  cktotal += (8+cksize);
		  if (match_four_chars((unsigned char *)hdrbuf,I_LIST))
		    {
		      ckoffr = ckoff+12;
		      cktotalr = 12;
		      while (cktotalr < cksize)
			{
			  lseek(chan,ckoffr,SEEK_SET);
			  read(chan,hdrbuf,8);
			  cksizer = get_little_endian_int((unsigned char *)(hdrbuf+4));
			  ckoffr += (8+cksizer);
			  cktotalr += (8+cksizer);
			  if (match_four_chars((unsigned char *)hdrbuf,I_strf))
			    {
			      if (cksizer < HDRBUFSIZ) rdsize = cksizer; else rdsize = HDRBUFSIZ;
			      read(chan,hdrbuf,rdsize);
			      original_sound_format = get_little_endian_short((unsigned char *)hdrbuf);
			      chans	= get_little_endian_short((unsigned char *)(hdrbuf+2));
			      srate = get_little_endian_int((unsigned char *)(hdrbuf+4));
			      /* block_align = get_little_endian_short((unsigned char *)(hdrbuf+12)); */
			      bits = get_little_endian_short((unsigned char *)(hdrbuf+14));
			      /* only 16 bit linear little endian for now */
			      if ((bits == 16) && (original_sound_format == 1))
				original_sound_format = snd_16_linear_little_endian;
			      if (data_location != 0) happy = 0;
			      break;
			    }
			}
		    }
		}
	    }
	}
      chunkloc = (8+chunksize);
      if (chunksize&1) chunkloc++; /* extra null appended to odd-length chunks */
    }
}



/* ------------------------------------ SoundFont 2.0 ------------------------------------
 *
 * Emu's SoundFont(tm) format uses RIFF -- at ftp.creaf.com:/pub/emu/sf2_00a.ps)
 *
 * RIFF xxxx sfbk followed by
 *   LIST xxxx INFO chunk (nothing of interest -- icmt subchunk might have comments)
 *   LIST xxxx sdta chunk = data
 *     smpl chunk (16 bit linear little-endian)
 *   LIST xxxx pdta list chunk 
 *     shdr subchunk has srate at 40 (int), samples at 28
 */

static void read_soundfont_header (int chan)
{
  /* we know we have checked for RIFF xxxx sfbk when we arrive here */
  int chunksize,offset,chunkloc,happy,type,cksize,rdsize,ckoff;
  char *str;
  type_specifier = get_uninterpreted_int((unsigned char *)(hdrbuf+8));
  chunkloc = 12;
  offset = 0;
  header_distributed = 1;
  sound_format = snd_16_linear_little_endian;
  srate = 0;
  chans = 1; /* to hell with soundfont stereo */
  happy = 1;
  data_size = 0;
  data_location = 0;
  true_file_length = lseek(chan,0L,SEEK_END);
  while (happy)
    {
      offset += chunkloc;
      if (seek_and_read(chan,(unsigned char *)hdrbuf,offset,32) <= 0)
	{
	  clm_printf("error reading header");
	  return;
	}
      chunksize = get_little_endian_int((unsigned char *)(hdrbuf+4));
      if (match_four_chars((unsigned char *)hdrbuf,I_LIST))
	{
	  /* everything is squirreled away in LIST chunks in this format */
	  if (match_four_chars((unsigned char *)(hdrbuf+8),I_pdta))
	    {
	      /* go searching for I_shdr -- headers this complicated should be illegal. */
	      ckoff = offset+12;
	      lseek(chan,ckoff,SEEK_SET);
	      while (!srate)
		{
		  read(chan,hdrbuf,8);
		  cksize = get_little_endian_int((unsigned char *)(hdrbuf+4));
		  ckoff += (8+cksize);
		  /* here we need to jump over subchunks! -- 4-Aug-97 */
		  if (match_four_chars((unsigned char *)hdrbuf,I_shdr))
		    {
		      if (cksize < HDRBUFSIZ) rdsize = cksize; else rdsize = HDRBUFSIZ;
		      read(chan,hdrbuf,rdsize);
		      data_size = 2 * get_little_endian_int((unsigned char *)(hdrbuf+24));
		      srate = get_little_endian_int((unsigned char *)(hdrbuf+36));
                      type = get_little_endian_short((unsigned char *)(hdrbuf+44));
		      if (type != 1) 
			{
			  str = (char *)CALLOC(64,sizeof(char));
			  sprintf(str,"can't handle this SoundFont data type: %x",type);
			  clm_printf(str);
			  FREE(str);
			  return;
			}
		      happy = (!data_location);
		    }
		  else lseek(chan,ckoff,SEEK_SET);
		}
	    }
	  else
	    {
	      if (match_four_chars((unsigned char *)(hdrbuf+8),I_sdta))
		{
		  /* assume smpl follows + subchunk size */
		  /* Convert 1.4 appears to create a separate smpl chunk */
		  data_location = offset+20; /* LIST xxxx sdta smpl xxxx ... */
		  happy = (!srate);
		}
	    }
	}
      chunkloc = (8+chunksize);
      if (chunksize&1) chunkloc++; /* extra null appended to odd-length chunks */
    }
  if (true_file_length < data_size) data_size = true_file_length - data_location;
  data_size = c_snd_samples(sound_format,data_size);
}




/* ------------------------------------ NIST ------------------------------------ 
 *
 * code available in ogitools-v1.0.tar.gz at svr-ftp.eng.cam.ac.uk:comp.speech/sources
 * 
 *   0: "NIST_1A"
 *   8: data_location as ASCII representation of integer (apparently always "   1024")
 *  16: start of complicated header -- see below for details
 *
 *  The most recent version of the SPHERE package is available
 *  via anonymous ftp from jaguar.ncsl.nist.gov [129.6.48.157] in the pub directory
 *  in compressed tar form as "sphere-v.tar.Z" (where "v" is the version
 *  code 2.6a last I looked).  shortpack is also at this site.
 *
 *  here's an example:
 *
 *  NIST_1A
 *     1024
 *  database_id -s5 TIMIT
 *  database_version -s3 1.0
 *  utterance_id -s8 aks0_sa1
 *  channel_count -i 1
 *  sample_count -i 63488
 *  sample_rate -i 16000
 *  sample_min -i -6967
 *  sample_max -i 7710
 *  sample_n_bytes -i 2
 *  sample_byte_format -s2 01
 *  sample_sig_bits -i 16
 *  end_head
 *
 * the sample_byte_format can be "10"=big-endian or "01"=little-endian, or "shortpack-v0"=compressed via shortpack
 * other formats are wavpack and shorten.
 *
 * another field is 'sample_coding' which can be pcm (i.e. linear), 'pcm,embedded-shorten-v1.09', mu-law, ulaw, pculaw etc --
 *   so unpredictable as to be totally useless. This means we sometimes try to decode shorten-encoded files because
 *   we ignore this field.  And worse, there's a 'channels_interleaved' field that (apparently) can be FALSE.  Tough.
 */

#define MAX_FIELD_LENGTH 80

static int decode_nist_value (char *str,int base,int end)
{
  /* can be -i -r or -snnn where nnn=ascii rep of integer = len of string (!) */
  /* we'll deal only with integer fields (and well-behaved string fields) */
  int i,j;
  char value[MAX_FIELD_LENGTH];
  i=base;
  while ((i<end) && (i<MAX_FIELD_LENGTH) && (str[i] != '-')) i++; /* look for -i or whatever */
  while ((i<end) && (i<MAX_FIELD_LENGTH) && (str[i] != ' ')) i++; /* look for space after it */
  i++;
  if (i>=MAX_FIELD_LENGTH) return(0);
  for (j=0;i<end;j++,i++)
    value[j]=str[i];
  value[j]=0;
  if (value[0]=='s') return(NIST_shortpack);
  sscanf(value,"%d",&i);
  return(i);
}

static void read_nist_header (int chan)
{
  char str[MAX_FIELD_LENGTH],name[MAX_FIELD_LENGTH];
  int happy = 1;
  int k,hend,curbase,j,n,nm,samples,bytes,byte_format;
  type_specifier = get_uninterpreted_int((unsigned char *)hdrbuf); /* the actual id is "NIST_1A" */
  for (k=8;k<16;k++) str[k-8]=hdrbuf[k];
  sscanf(str,"%d",&data_location);       /* always "1024" */
  n = 16;
  hend = INITIAL_READ_SIZE;
  k=0;
  curbase = 0;
  samples = 0;
  bytes = 0;
  srate = 0;
  chans = 0;
  comment_start = 16;
  comment_end = 16;
  byte_format = 10;
  for (j=0;j<MAX_FIELD_LENGTH;j++) str[j]=' ';  
  while (happy) 
    {
      /* much as in xIFF files, march through the file looking for the data we're after */
      /* in this case we munch a character at a time... */
      str[k] = hdrbuf[n];
      if ((((str[k] == '\0') || (str[k] == '\n')) || ((curbase+n+1) >= data_location)) || (k == 79))
	{
	  /* got a complete record (assuming no embedded newlines, of course) */
	  /* now look for a record we care about and decode it */
	  nm = 0;
	  while ((str[nm] != ' ') && (nm < MAX_FIELD_LENGTH))
	    {
	      name[nm] = str[nm];
	      nm++;
	    }
	  if (nm >= MAX_FIELD_LENGTH) {header_type = raw_sound_file; sound_format = snd_unsupported; return;}
	  name[nm]=0;
	  if (strcmp(name,"sample_rate") == 0) srate = decode_nist_value(str,nm,k); else
	    if (strcmp(name,"channel_count") == 0) chans = decode_nist_value(str,nm,k); else
	      if (strcmp(name,"end_head") == 0) {happy = 0; comment_end=curbase+n-1;} else
		if (strcmp(name,"sample_count") == 0) samples = decode_nist_value(str,nm,k); else
		  if ((bytes == 0) && (strcmp(name,"sample_n_bytes") == 0)) bytes = decode_nist_value(str,nm,k); else
		    if ((bytes == 0) && (strcmp(name,"sample_sig_bits") == 0)) {bytes = decode_nist_value(str,nm,k); bytes = (bytes>>3);} else
		      if (strcmp(name,"sample_byte_format") == 0) byte_format = decode_nist_value(str,nm,k); else
			if (strcmp(name,"end_head") == 0) happy = 0;
	  for (j=0;j<=k;j++) str[j]=' ';
	  k=0;
	  if ((curbase+n+1) > 1024) happy=0;
	}
      else
	k++;
      n++;
      if (n >= hend)
	{
	  curbase += hend;
	  n = 0;
	  read(chan,hdrbuf,HDRBUFSIZ);
	  hend = HDRBUFSIZ;
	}
    }
  data_size = samples*bytes;
  if (byte_format == NIST_shortpack)
    {
      sound_format = snd_unsupported;
      original_sound_format = NIST_shortpack;
    }
  else
    {
      if (bytes == 2) 
	{
	  if (byte_format == 10) 
	    sound_format = snd_16_linear;
	  else sound_format = snd_16_linear_little_endian;
	}
      else sound_format = snd_8_mulaw;
    }
  data_size = c_snd_samples(sound_format,data_size);
  header_distributed = 1;
}

static void write_nist_header (int chan, int srate, int chans, int siz, int format)
{
  char *header;
  int datum;
  datum = c_snd_datum_size(format);
  header = (char *)calloc(1024,sizeof(char));
  sprintf(header,"NIST_1A\n   1024\nchannel_count -i %d\nsample_rate -i %d\nsample_n_bytes -i %d\nsample_byte_format -s2 %s\nsample_sig_bits -i %d\nsample_count -i %d\nend_head\n",
	  chans,srate,datum,
	  ((format == snd_16_linear) || (format == snd_24_linear) || (format == snd_32_linear)) ? "10" : "01",
	  datum* 8,siz/datum);
#ifndef MACOS
  write(chan,(unsigned char *)header,1024);
#else
  write(chan,header,1024);
#endif
  data_location = 1024;
  free(header);
}

static void update_nist_header (int chan, int siz)
{
  lseek(chan,0L,SEEK_SET);
  read(chan,hdrbuf,INITIAL_READ_SIZE);
  read_nist_header(chan);
  lseek(chan,0L,SEEK_SET);
  write_nist_header(chan,c_snd_header_srate(),c_snd_header_chans(),siz,c_snd_header_format());
}


/* ------------------------------------ BICSF ------------------------------------ 
 * (actually, this is EBICSF and the old BICSF is called IRCAM below)
 *
 * 0-28: NeXT-compatible header, read by read_next_header above.
 *   28: bicsf magic number (107364 or trouble)
 *   32: srate as a 32-bit float
 *   36: chans
 *   40: data format indicator (2=16-bit linear, 4=32-bit float)
 *   44: begin chunks, if any
 *
 * followed by AIFF-style chunked header info with chunks like:
 *
 *   COMM size comment
 *   MAXA size {max amps (up to 4)} (frame offsets) time-tag unix msec counter
 *   CUE, PRNT, ENV etc 
 */

static void read_bicsf_header (int chan)
{
  int chunksize,chunkname,offset,chunkloc,happy;
  type_specifier = get_uninterpreted_int((unsigned char *)(hdrbuf+28));
  header_type = BICSF_sound_file;
  data_location = 1024;
  if (data_size == 0) data_size = (true_file_length-data_location);
  lseek(chan,40,SEEK_SET);
  read(chan,hdrbuf,HDRBUFSIZ);
  original_sound_format = get_big_endian_int((unsigned char *)hdrbuf);
  switch (original_sound_format) 
    {
    case 2: sound_format = snd_16_linear; break;
    case 4: sound_format = snd_32_float; break;
    default: break;
    }

  /* now check for a COMM chunk, setting the comment pointers */
  chunkloc = 4; /* next header + magic number, srate, chans, packing, then chunks, I think */
  offset = 40;
  header_distributed = 1;
  happy = 1;
  while (happy)
    {
      if ((offset+chunkloc) >= data_location) 
	happy = 0;
      else
	{
	  offset += chunkloc;
	  if (seek_and_read(chan,(unsigned char *)hdrbuf,offset,32) <= 0)
	    {
	      clm_printf("error reading header");
	      return;
	    }
	  chunkname = get_uninterpreted_int((unsigned char *)hdrbuf);
	  chunksize = get_big_endian_int((unsigned char *)(hdrbuf+4));
	  if (match_four_chars((unsigned char *)hdrbuf,I_COMM))
	    {
	      comment_start = 8+offset;
	      comment_end = comment_start + chunksize -1;
	      happy = 0;
	    }
	  else
	    {
	      if ((chunkname == 0) || (chunksize == 0)) 
		happy = 0;
	    }
	  chunkloc = (8+chunksize);
	}
    }
  /* from here we fall back into read_next_header */
}



/* ------------------------------------ IRCAM ------------------------------------ 
 * read/write CLM (old-style BICSF) -- added write option for Sun port 12-Dec-94
 *
 *    0: 0x1a364 or variations thereof -- byte order gives big/little_endian decision,
 *         ^ digit gives machine info, according to AFsp sources -- see IRCAM ints above
 *    4: srate as a 32-bit float
 *    8: chans
 *   12: data format indicator (2=16-bit linear, 4=32-bit float)
 *       according to new Sox (version 11), these packing modes are now bytes/sample in low short, code in high
 *       so 1 = char, 0x10001 = alaw, 0x20001 = mulaw, 2 = short, 0x40004 = long, 4 = float
 *   16: comment start -- how to tell if it's a real comment?
 *       apparently these are separated as short code, short blocksize, then data
 *       codes: 0=end, 1=maxamp, 2=comment, 3=pvdata, 4=audioencode and codemax??
 * 1024: data start
 * 
 * apparently the byte order depends on the machine.
 * and yet... convert 1.4 makes a .sf file with little endian header, the VAX id, and big endian data?
 */

static void read_ircam_header (int chan)
{
  short bcode,bloc,bsize;
  int happy,offset,little;
  type_specifier = get_uninterpreted_int((unsigned char *)hdrbuf);
  if ((get_little_endian_int((unsigned char *)hdrbuf) == I_IRCAM_VAX) || 
      (get_little_endian_int((unsigned char *)hdrbuf) == I_IRCAM_MIPS))
    little=1;
  else little=0;
  data_location = 1024;
  true_file_length=lseek(chan,0L,SEEK_END);
  data_size = (true_file_length-1024);
  original_sound_format = big_or_little_endian_int((unsigned char *)(hdrbuf+12),little);
  sound_format = snd_unsupported;
  if (original_sound_format == 2) 
    {
      if (little) sound_format = snd_16_linear_little_endian; else sound_format = snd_16_linear;
    }
  else if (original_sound_format == 4) 
    {
      if (little) 
	{
	  if (get_little_endian_int((unsigned char *)hdrbuf) == I_IRCAM_VAX)
	    sound_format = snd_32_vax_float;
	  else sound_format = snd_32_float_little_endian;
	}
      else sound_format = snd_32_float;
    }
  else if (original_sound_format == 0x40004) 
    {
      if (little) sound_format = snd_32_linear_little_endian;
      else sound_format = snd_32_linear;
    }
  else if (original_sound_format == 0x10001) sound_format = snd_8_alaw;
  else if (original_sound_format == 0x20001) sound_format = snd_8_mulaw;
  else if (original_sound_format == 1) sound_format = snd_8_linear;
  srate = (int)big_or_little_endian_float((unsigned char *)(hdrbuf+4),little);
  chans = big_or_little_endian_int((unsigned char *)(hdrbuf+8),little);
  bloc=16;
  happy=1;
  offset=0;
  while (happy)
    {
      offset += bloc;
      if (seek_and_read(chan,(unsigned char *)hdrbuf,offset,32) <= 0)
	{
	  clm_printf("error reading header");
	  return;
	}
      bcode = big_or_little_endian_short((unsigned char *)hdrbuf,little);
      bsize = big_or_little_endian_short((unsigned char *)(hdrbuf+2),little);
      if (bcode == 2)
	{
	  happy = 0;
	  comment_start = 4+offset;
	  comment_end = comment_start+bsize-1; /* was -5? */
	}
      bloc = bsize;
      if ((bsize <= 0) || (bcode <= 0) || ((offset+bloc) > 1023)) happy = 0;
    }
  header_distributed = 0;
  data_size = c_snd_samples(sound_format,data_size);
}

static void write_ircam_header (int chan, int srate, int chans, int format, char *comment, int len)
{
  int i,j;
  char *str;
#ifdef NEXT
  set_big_endian_int((unsigned char *)hdrbuf,0x4a364);
#else
  set_big_endian_int((unsigned char *)hdrbuf,0x2a364); /* SUN id */
#endif
  set_big_endian_float((unsigned char *)(hdrbuf+4),(float)srate);
  set_big_endian_int((unsigned char *)(hdrbuf+8),chans);
  switch (format)
    {
    case snd_8_mulaw: set_big_endian_int((unsigned char *)(hdrbuf+12),0x20001); break;
    case snd_8_alaw: set_big_endian_int((unsigned char *)(hdrbuf+12),0x10001); break;
    case snd_16_linear: set_big_endian_int((unsigned char *)(hdrbuf+12),2); break;
    case snd_32_linear: set_big_endian_int((unsigned char *)(hdrbuf+12),0x40004); break;
    case snd_32_float: set_big_endian_int((unsigned char *)(hdrbuf+12),4); break;
    default: 
      str=(char *)CALLOC(256,sizeof(char));
      sprintf(str,"IRCAM unsupported sound data format type: %d\n",format);
      clm_printf(str);
      FREE(str);
      break;
    }
  if (len > 0)
    {
      set_big_endian_short((unsigned char *)(hdrbuf+16),2);
      set_big_endian_short((unsigned char *)(hdrbuf+18),(short)len);
    }
  else
    {
      set_big_endian_int((unsigned char *)(hdrbuf+16),0);
    }
  write(chan,hdrbuf,20);
  data_location = 1024;
  j = 0;
  for (i=0;i<len;i++) 
    {
      hdrbuf[j]=comment[i];
      j++;
      if (j == HDRBUFSIZ) 
	{
	  write(chan,hdrbuf,HDRBUFSIZ);
	  j = 0;
	}
    }
  for (i=0;i<(1024-(len+20));i++) /* now fill left over bytes with nulls */
    {
      hdrbuf[j]=0;
      j++;
      if (j == HDRBUFSIZ) 
	{
	  write(chan,hdrbuf,HDRBUFSIZ);
	  j = 0;
	}
    }
  if (j != 0) write(chan,hdrbuf,j);
}

static void update_ircam_header (void)
{
  /* size is implicit in file size - header */
}


/* ------------------------------------ 8SVX ------------------------------------- 
 * (also known as IFF)
 *
 * very similar to AIFF:
 *  "BODY" => [4] samples [n] data
 *  "VHDR" => srate (short)
 *  "CHAN" => chans
 *  "ANNO" and "NAME"
 *
 * big_endian throughout
 */

static void read_8svx_header (int chan)
{
  int chunksize,offset,chunkloc,happy;
  type_specifier = get_uninterpreted_int((unsigned char *)hdrbuf);
  chunkloc = 12;
  offset = 0;
  sound_format = snd_8_linear;
  header_distributed = 1;
  srate = 0;
  chans = 1;
  happy = 1;
  update_form_size = get_big_endian_int((unsigned char *)(hdrbuf+4));
  while (happy)
    {
      offset += chunkloc;
      if (seek_and_read(chan,(unsigned char *)hdrbuf,offset,32) <= 0)
	{
	  clm_printf("error reading header");
	  return;
	}
      chunksize = get_big_endian_int((unsigned char *)(hdrbuf+4));
      if (match_four_chars((unsigned char *)hdrbuf,I_CHAN))
	{
	  chans = get_big_endian_int((unsigned char *)(hdrbuf+8));
	  chans = (chans & 0x01) + ((chans & 0x02) >> 1) + ((chans & 0x04) >> 2) + ((chans & 0x08) >> 3);
	  /* what in heaven's name is this?  Each bit corresponds to a channel? */
	}
      else
	{
	  if (match_four_chars((unsigned char *)hdrbuf,I_VHDR))
	    {
	      /* num_samples (int) at hdrbuf+8 */
	      srate = get_big_endian_unsigned_short((unsigned char *)(hdrbuf+20));
	      original_sound_format = hdrbuf[23];
	      if (original_sound_format != 0) sound_format = snd_unsupported;
	    }
	  else
	    {
	      if ((match_four_chars((unsigned char *)hdrbuf,I_ANNO)) || (match_four_chars((unsigned char *)hdrbuf,I_NAME)))
		{
		  comment_start = offset+8;
		  comment_end = comment_start+chunksize-1;
		}
	      else
		{
		  if (match_four_chars((unsigned char *)hdrbuf,I_BODY))
		    {
		      data_size = chunksize;
		      data_location = offset+12;
		      happy = 0;
		    }
		}
	    }
	}
      chunkloc = (8+chunksize);
      if (chunksize&1) chunkloc++; /* extra null appended to odd-length chunks */
    }
  data_size = c_snd_samples(sound_format,data_size);
}


/* ------------------------------------ VOC -------------------------------------- 
 *
 *   0: "Creative Voice File" followed by a couple ctrl-Z ('32) (swapped data)
 *  20: header end (short) {8svx, 26=data_offset, 0x10a=version, ((~version + 0x1234) & 0xffff) = 0x1129}
 * [20]: first block:
 *     block code, 1=data, 0=end, 9=data_16 (2=continue, 3=silence, 4=marker, 5=text, 6=loop, 7=loop-end, 8=extended)
 *     block len as 24 bit int(?)
 *     if data, then rate code (byte), then data (assuming 8-bit unsigned, mono)
 *     if data_16, long srate, byte: data size (8 or 16), byte chans
 *     if text, ascii text (a comment)
 *     if extended (8) precedes 1 (data): 8 4 then time constant (short), byte: packing code (0), byte chans (0=mono)
 *
 * apparently always little_endian
 * updated extensively 29-Aug-95 from sox10 voc.c
 */

static void read_voc_header(int chan)
{
  int type,happy,len,curbase,voc_extended,bits,code;
  sound_format = snd_8_unsigned;
  chans = 1;
  happy = 1;
  voc_extended = 0;
  true_file_length=lseek(chan,0L,SEEK_END);
  curbase = get_little_endian_short((unsigned char *)(hdrbuf+20));
  header_distributed = 1;
  lseek(chan,curbase,SEEK_SET);
  read(chan,hdrbuf,HDRBUFSIZ);
  type = (int)(hdrbuf[0]);
  len=(((int)hdrbuf[3])<<16)+(((int)hdrbuf[2])<<8)+(((int)hdrbuf[1]));
  while (happy)
    {
      if (type == 1) /* voc_data */
	{
	  data_size = len-1; /* was -3 */
	  data_location = curbase+6;
	  if (voc_extended == 0) 
	    {
	      srate = (int)(1000000.0/(256 - ((int)(hdrbuf[4]&0xff))));
	      original_sound_format = hdrbuf[5];
	      if (hdrbuf[5] == 0) sound_format = snd_8_unsigned; else sound_format = snd_unsupported;
	    }
	  happy = 0;
	}
      else
	{
	  if (type == 9) /* voc_data_16 */
	    {
	      data_size = len-1; /* was -3 */
	      data_location = curbase+6;
	      srate = get_little_endian_int((unsigned char *)(hdrbuf+4));
	      bits = ((int)hdrbuf[8]);
	      if (bits == 8)
		{
		  code = get_little_endian_short((unsigned char *)(hdrbuf+10));
		  if (code == 6) 
		    sound_format = snd_8_alaw;
		  else
		    if (code == 7)
		      sound_format = snd_8_mulaw;
		    else sound_format = snd_8_unsigned; 
		}
	      else 
		if (bits == 16) 
		  sound_format = snd_16_linear_little_endian;
		else sound_format = snd_unsupported;
	      chans = (int)hdrbuf[9];
	      if (chans == 0) chans = 1;
	      happy = 0;
	    }
	  else
	    {
	      if (((len+curbase)<true_file_length) && (type != 0))
		{
		  if (type == 5) /* voc_text */
		    {
		      comment_start = curbase+4;
		      comment_end = comment_start + len - 1;
		    }
		  else
		    {
		      if (type == 8) /* voc_extended */
			{
			  srate = (256000000 / (65536 - get_little_endian_short((unsigned char *)(hdrbuf+4))));
			  if ((int)(hdrbuf[7]) == 0) chans=1; else chans=2;
			  if ((int)(hdrbuf[6]) != 0) sound_format = snd_unsupported;
			}
		    }
		  if (seek_and_read(chan,(unsigned char *)hdrbuf,curbase+len+4,HDRBUFSIZ) <= 0)
		    {
		      clm_printf("error reading header");
		      return;
		    }
		  curbase += len;
		}
	      else happy = 0;
	    }
	}
    }
  data_size = c_snd_samples(sound_format,data_size);
}



/* ------------------------------------ ADC ------------------------------------ 
 * also known as OGI format
 * TIMIT format is identical except it omits the data format field (header size claims to be bytes)
 *
 * from ad.h and other files, ogitools-v1.0.tar.gz
 * we'll look for the big/little endian sequence (short) 8 1 1-or-2 given big/little decision
 *
 * 0: header size in shorts (8 = 16 bytes) (OGI says this is in bytes)
 * 2: version (1)
 * 4: chans
 * 6: rate (srate = 4000000/rate)
 * 8: samples (int) -- seems to be off by 2 -- are they counting ints here?
 * 12: data format (0=big-endian)
 * 16: data start
*/ 

static void read_adc_header(int chan)
{
  int little;
  little = get_uninterpreted_int((unsigned char *)(hdrbuf+12)); /* 0=big endian */
  data_location = 16;
  if (little) sound_format = snd_16_linear_little_endian; else sound_format = snd_16_linear;
  chans = big_or_little_endian_short((unsigned char *)(hdrbuf+4),little);
  srate = 4000000 / big_or_little_endian_short((unsigned char *)(hdrbuf+6),little);
  data_size = 2*big_or_little_endian_int((unsigned char *)(hdrbuf+8),little);
  comment_start=0;
  comment_end=0;
  header_distributed = 0;
  true_file_length=lseek(chan,0L,SEEK_END);
  data_size = c_snd_samples(sound_format,data_size);
}



/* ------------------------------------ AVR -------------------------------------- 
 *
 *   0: "2BIT"
 *   4: sample name (null padded ASCII)
 *  12: chans (short) (0=mono, -1=stereo)
 *  14: sample size (8 or 16 bit) (short) (value is 8 or 16)
 *  16: sample format (signed or unsigned) (short) (0=unsigned, -1=signed)
 *  18: loop (on/off), 20: midi (-1 = ?)
 *  22: srate 
 *      avr.txt has:
 *      22: Replay speed 0=5.485 Khz, 1=8.084 Khz, 2=10.971 Khz, 3=16.168 Khz, 4=21.942 Khz, 5=32.336 Khz, 6=43.885 Khz, 7=47.261 Khz
 *      23: sample rate	in Hertz (as a 3 byte quantity??)
 *  26: length in samples
 *  30: loop beg, 34: loop end, 38: midi (keyboard split), 40: compression, 42: nada ("reserved"), 44: name
 *  64: comment (limited to 64 bytes)
 * 128: data start
 *
 * the Atari .avr files appear to be 8000 Hz, mono, 8-bit linear unsigned data with an unknown header of 128 words
 * apparently there was a change in format sometime in the 90's.
 * 
 * The actual avr files I've found on the net are either garbled, or
 * something is wrong with this definition (taken from CMJ). 
 * SGI dmconvert assumes big-endian here -- this is an Atari format, so it's probably safe to assume big-endian.
 */

static void read_avr_header(int chan)
{
  int dsize,dsigned,i;
  chans = get_big_endian_short((unsigned char *)(hdrbuf+12));
  if (chans == 0) chans=1; else chans=2;
  data_location = 128;
  data_size = get_big_endian_int((unsigned char *)(hdrbuf+26));
  header_distributed = 0;
  srate = get_big_endian_unsigned_short((unsigned char *)(hdrbuf+24));
  dsize = get_big_endian_short((unsigned char *)(hdrbuf+14));
  dsigned = get_big_endian_short((unsigned char *)(hdrbuf+16));
  if (dsize == 16) 
    {
      if (dsigned == 0)
	sound_format = snd_16_unsigned;
      else sound_format = snd_16_linear;
    }
  else 
    {
      if (dsigned == 0) 
	sound_format = snd_8_unsigned;
      else sound_format = snd_8_linear;
    }
  if (seek_and_read(chan,(unsigned char *)hdrbuf,64,64) <= 0)
    {
      clm_printf("error reading header");
      return;
    }
  comment_start = 64;
  i = 0;
  while ((i < 64) && (hdrbuf[i] != 0)) i++;
  comment_end = 64+(i-1);
  true_file_length=lseek(chan,0L,SEEK_END);
  data_size = c_snd_samples(sound_format,data_size);
}




/* ------------------------------------ SNDT -------------------------------------
 *
 * this taken from sndrtool.c (sox-10): (modified 6-Feb-98)
 *   0: "SOUND" (or off by two throughout if not "0x1a"?)
 *   5: 0x1a
 *   6-7: 0
 *   8-11: nsamps (at 12)
 *  12-15: 0
 *  16-19: nsamps
 *  20-21: srate (little endian short) (at 22)
 *  22-23: 0 
 *  24-25: 10
 *  26-27: 4
 *  28-> : <filename> "- File created by Sound Exchange"
 *  .->95: 0 ?
 */

/* similar is Sounder format: 
 * 0: 0
 * 2: short srate (little endian)
 * 4: 10
 * 6: 4
 * then data
 * but this format can't be distinguished from a raw sound file
 */

static void read_sndt_header(int chan)
{
  sound_format = snd_8_unsigned;
  chans = 1;
  srate = get_little_endian_unsigned_short((unsigned char *)(hdrbuf+20));
  data_location = 126;
  data_size = get_little_endian_int((unsigned char *)(hdrbuf+8));
  if (data_size < 0) data_size = get_little_endian_int((unsigned char *)(hdrbuf+10));
  if (srate <= 1) srate = get_little_endian_unsigned_short((unsigned char *)(hdrbuf+22));
  header_distributed = 0;
  true_file_length=lseek(chan,0L,SEEK_END);
}


/* ------------------------------------ Covox v8 ------------------------------------- 
 *
 *  0: 377 125 377 252 377 125 377 252 x x 0's to 16
 * then 8-bit unsigned data
 */

static void read_covox_header(int chan)
{
  sound_format = snd_8_unsigned;
  chans = 1;
  header_distributed = 0;
  data_location = 16;
  srate = 8000;
  true_file_length=lseek(chan,0L,SEEK_END);
  data_size = true_file_length - data_location;
}


/* ------------------------------------ Digitracker SPL ------------------------------------- 
 * (Atari-related, I think, and now obsolete)
 * 0: DSPL
 * 4: version
 * 5: sample name
 * 37: filename
 * 45: c-4 freq (srate?)
 * 47: length
 * 51: loop data
 * 59: vol
 * 60: bit 0: 0=8 bit, 1=16
 * 61: data (in sample data chunks or as straight data??)
 */

static void read_spl_header(int chan)
{
  chans = 1;
  header_distributed = 0;
  data_location = 61;
  srate = 8000; /* I need an example to decode this */
  true_file_length=lseek(chan,0L,SEEK_END);
  if (hdrbuf[60]&1) sound_format = snd_16_linear; else sound_format = snd_8_linear; /* unsigned? */
  data_size = c_snd_samples(sound_format,true_file_length - data_location);
}


/* ------------------------------------ SMP ------------------------------------- 
 *
 *  0: "SOUND SAMPLE DATA "
 * 18: "2.1 "
 * 22-81: comment
 * 82-111: sample name
 * header 112 bytes
 * long samples (bytes=samples*2)
 * then data start
 * data
 * always little endian
 */

static void read_smp_header(int chan)
{
  sound_format = snd_16_linear_little_endian;
  chans = 1;
  comment_start=22;
  comment_end=81;
  header_distributed = 1;
  data_location = 116;
  lseek(chan,112,SEEK_SET);
  read(chan,hdrbuf,4);
  data_size = 2*(get_little_endian_int((unsigned char *)hdrbuf));
  srate = 8000; /* docs mention an srate floating around at the end of the file, but I can't find it in any example */
  true_file_length=lseek(chan,0L,SEEK_END);
  data_size = c_snd_samples(sound_format,data_size);
}


/* ------------------------------------ SPPACK ------------------------------------- 
 * 
 * from AF docs:
 *         Bytes   Type    Contents
 *     0   160    char   Text strings (2 * 80)
 *   160    80    char   Command line
 *   240     2    int    Domain
 *   242     2    int    Frame size
 *   244     4    float  Sampling frequency
 *   252     2    int    File identifier (i.e. #o100 #o303)
 *   254     2    int    Data type (0xfc0e = sampled data file)
 *   256     2    int    Resolution (in bits)
 *   258     2    int    Companding flag (1=linear, 2=alaw, 3=mulaw)
 *   272   240    char   Text strings (3 * 80)
 *   512   ...    --     Audio data
 *
 */

static void read_sppack_header(int chan)
{
  int typ,bits;
  float sr;
  data_location = 512;
  chans = 1;
  lseek(chan,240,SEEK_SET);
  read(chan,hdrbuf,22);
  typ = get_big_endian_short((unsigned char *)hdrbuf);
  sound_format = snd_unsupported;
  header_distributed = 0;
  if (typ == 1) 
    {
      if (((hdrbuf[254])==252) && ((hdrbuf[255])==14)) /* #xfc and #x0e */
	{
	  typ = get_big_endian_short((unsigned char *)(hdrbuf+18));
	  bits = get_big_endian_short((unsigned char *)(hdrbuf+16));
	  sr = get_big_endian_float((unsigned char *)(hdrbuf+4));
	  srate = (int)sr;
	  switch (typ)
	    {
	    case 1: if (bits == 16) sound_format = snd_16_linear; else sound_format = snd_8_linear; break;
	    case 2: sound_format = snd_8_alaw; break;
	    case 3: sound_format = snd_8_mulaw; break;
	    default: sound_format = snd_unsupported; break;
	    }
	  data_size=lseek(chan,0L,SEEK_END);
	  data_size=c_snd_samples(sound_format,data_size-512);
	  comment_start=0;
	  comment_end=0;
	}
    }
  true_file_length=lseek(chan,0L,SEEK_END);
}


/* ------------------------------------ ESPS (Entropic Signal Processing System) ------------------------------------- 
 *
 * specs at ftp.entropic.com
 * from AFgetInfoES.c:
 * 
 *       Bytes     Type    Contents
 *      8 -> 11    --     Header size (bytes)
 *     12 -> 15    int    Sampled data record size
 *     16 -> 19    int    File identifier: 0x00006a1a or 0x1a6a0000
 *     40 -> 65    char   File creation date
 *    124 -> 127   int    Number of samples
 *    132 -> 135   int    Number of doubles in a data record
 *    136 -> 139   int    Number of floats in a data record
 *    140 -> 143   int    Number of longs in a data record
 *    144 -> 147   int    Number of shorts in a data record
 *    148 -> 151   int    Number of chars in a data record
 *    160 -> 167   char   User name 
 *    333 -> H-1   --     "Generic" header items, including "record_freq" {followed by a "double8"=64-bit ?}
 *      H -> ...   --     Audio data
 */

static void read_esps_header (int chan)
{
  char str[80];
  int happy = 1;
  int k,hend,curbase,j,n,chars,floats,shorts,doubles,little;
  data_location = get_big_endian_int((unsigned char *)(hdrbuf+8));
  little = (hdrbuf[18] == 0);
  true_file_length = lseek(chan,0L,SEEK_END);
  data_size = true_file_length - data_location;
  header_distributed = 0;
  srate = 8000;
  chans = 1;
  lseek(chan,132,SEEK_SET);
  read(chan,hdrbuf,HDRBUFSIZ);
  if (little)
    {
      doubles = get_little_endian_int((unsigned char *)hdrbuf);
      floats = get_little_endian_int((unsigned char *)(hdrbuf+4));
      shorts = get_little_endian_int((unsigned char *)(hdrbuf+12));
      chars = get_little_endian_int((unsigned char *)(hdrbuf+16));
    }
  else
    {
      doubles = get_big_endian_int((unsigned char *)hdrbuf);
      floats = get_big_endian_int((unsigned char *)(hdrbuf+4));
      shorts = get_big_endian_int((unsigned char *)(hdrbuf+12));
      chars = get_big_endian_int((unsigned char *)(hdrbuf+16));
    }
  if (shorts != 0)
    {
      sound_format = ((little) ? snd_16_linear_little_endian : snd_16_linear); 
      chans = shorts;
    }
  else
    {
      if (doubles != 0)
	{
	  sound_format = ((little) ? snd_64_double_little_endian : snd_64_double);
	  chans = doubles;
	}
      else
	{
	  if (floats != 0)
	    {
	      sound_format = ((little) ? snd_32_float_little_endian : snd_32_float);
	      chans = floats;
	    }
	  else
	    {
	      if (chars != 0)
		{
		  sound_format = snd_8_linear; /* ?? */
		  chans = chars;
		}
	    }
	}
    }
  /* search for "record_freq" to get srate */
  lseek(chan,333,SEEK_SET);
  read(chan,hdrbuf,HDRBUFSIZ);
  curbase=333;
  hend=curbase+HDRBUFSIZ;
  k=0;
  n=0;
  for (j=0;j<80;j++) str[j]=' ';  
  while (happy) 
    {
      str[k] = hdrbuf[n];
      if ((str[k] == 'q') || (str[k] == 3) || ((curbase+n+1) >= data_location) || (k == 78))
	{ /* 3 = C-C marks end of record */
	  str[k+1]=0;
	  if (strcmp(str,"record_freq") == 0) 
	    {
	      if (seek_and_read(chan,(unsigned char *)hdrbuf,curbase+n,32) <= 0)
		{
		  clm_printf("error reading header");
		  return;
		}
	      n=0;
	      if (little)
		srate = (int)get_little_endian_double((unsigned char *)(hdrbuf+8));
	      else srate = (int)get_big_endian_double((unsigned char *)(hdrbuf+8));
	      happy = 0;
	    }
	  if ((curbase+n+1) >= data_location) happy = 0;
	  k=0;
	}
      else
	k++;
      n++;
      if (n >= hend)
	{
	  curbase += hend;
	  n = 0;
	  read(chan,hdrbuf,HDRBUFSIZ);
	  hend = HDRBUFSIZ;
	}
    }
  if (srate == 0) srate = 8000;
  data_size = c_snd_samples(sound_format,data_size);
}

 
 
/* ------------------------------------ INRS ------------------------------------- 
 * 
 *   from AFgetInfoIN.c:
 * 
 *    INRS-Telecommunications audio file:
 *       Bytes     Type    Contents
 *      0 ->  3    float  Sampling Frequency (VAX float format)
 *      6 -> 25    char   Creation time (e.g. Jun 12 16:52:50 1990)
 *     26 -> 29    int    Number of speech samples in the file (? -- old INRS files omit this)
 *   The data in an INRS-Telecommunications audio file is in 16-bit integer (little-endian)
 *   format. Header is always 512 bytes.  Always mono.
 * 
 */

static int inrs_srates[NINRS] = {6500, 6667, 8000, 10000, 12000, 16000, 20000};

static void read_inrs_header (int chan, int loc)
{

  true_file_length = lseek(chan,0L,SEEK_END);
  comment_start = 6;
  comment_end = 25;
  header_distributed = 0;
  sound_format = snd_16_linear_little_endian;
  srate = loc;
  chans = 1;
  data_location = 512;
  data_size = c_snd_samples(sound_format,true_file_length-512);
}


/* ------------------------------------ MAUD ------------------------------------- 
 *
 * very similar to AIFF:
 *  "MHDR" => 4: chunksize (32)
 *            8: samples 
 *           12: bits 
 *           14: ditto
 *           16: clock freq
 *           20: clock div (srate = freq/div)
 *           22: chan info (0=mono, 1=stereo)
 *           24: ditto(?!)
 *           26: format (0=unsigned 8 or signed 16 (see bits), 2=alaw, 3=mulaw)
 *           28-40: unused
 *  "MDAT" => data
 *  "ANNO" => comment
 */

static void read_maud_header (int chan)
{
  int chunksize,offset,chunkloc,happy,num,den;
  type_specifier = get_uninterpreted_int((unsigned char *)hdrbuf);
  chunkloc = 12;
  offset = 0;
  sound_format = snd_8_linear;
  header_distributed = 1;
  srate = 0;
  chans = 1;
  happy = 1;
  update_form_size = get_big_endian_int((unsigned char *)(hdrbuf+4));
  while (happy)
    {
      offset += chunkloc;
      if (seek_and_read(chan,(unsigned char *)hdrbuf,offset,32) <= 0)
	{
	  clm_printf("error reading header");
	  return;
	}
      chunksize = get_big_endian_int((unsigned char *)(hdrbuf+4));
      if (match_four_chars((unsigned char *)hdrbuf,I_MHDR))
	{
	  data_size = get_big_endian_int((unsigned char *)(hdrbuf+8));
	  num = get_big_endian_int((unsigned char *)(hdrbuf+16));
	  den = get_big_endian_short((unsigned char *)(hdrbuf+20));
	  srate = (int)(num/den);
	  num = get_big_endian_short((unsigned char *)(hdrbuf+12));
	  den = get_big_endian_short((unsigned char *)(hdrbuf+26));
	  if (num == 8)
	    {
	      switch (den)
		{
		case 0: sound_format = snd_8_unsigned; break;
		case 2: sound_format = snd_8_alaw; break;
		case 3: sound_format = snd_8_mulaw; break;
		default: sound_format = snd_unsupported; break;
		}
	    }
	  else sound_format = snd_16_linear;
	  num = get_big_endian_short((unsigned char *)(hdrbuf+22));
	  if (num == 0) chans = 1; else chans = 2;
	}
      else
	{
	  if (match_four_chars((unsigned char *)hdrbuf,I_ANNO))
	    {
	      comment_start = offset+8;
	      comment_end = comment_start+chunksize-1;
	    }
	  else
	    {
	      if (match_four_chars((unsigned char *)hdrbuf,I_MDAT))
		{
		  data_location = offset+12;
		  happy = 0;
		}
	    }
	}
      chunkloc = (8+chunksize);
      if (chunksize&1) chunkloc++; /* extra null appended to odd-length chunks */
    }
  data_size = c_snd_samples(sound_format,data_size);
}



/* ------------------------------------ Sound Designer I -------------------------------------
 *
 * complicated and defined in terms of Pascal records, so the following is a stab in the dark:
 *
 * 0:    1336 (i.e. header size)
 * 764:  comment (str255)
 * 1020: sample rate (long)
 * 1028: data size (short)
 * 1030: a code string describing the data type (i.e. "linear") (str32)
 * 1064: user comment (str255)
 *
 * file type: 'SFIL'
 *
 * always big_endian
 */

static void read_sd1_header (int chan)
{
  int n;
  chans = 1;
  header_distributed = 0;
  data_location = 1336;
  lseek(chan,1020,SEEK_SET);
  read(chan,hdrbuf,64);
  srate = get_big_endian_int((unsigned char *)hdrbuf);
  n = get_big_endian_short((unsigned char *)(hdrbuf+8));
  if (n == 16)
    sound_format = snd_16_linear;
  else sound_format = snd_8_linear;
  true_file_length = lseek(chan,0L,SEEK_END);
  data_size = c_snd_samples(sound_format,true_file_length-1336);
  n = ((unsigned char)hdrbuf[44]);
  if (n != 0) 
    {
      comment_start = 1064;
      comment_end = comment_start+n-1;
    }
}



/* ------------------------------------ PSION alaw -------------------------------------
 *
 * 0: "ALawSoundFile**"
 * 16: version 
 * 18: length (bytes)
 * 22: padding
 * 24: repeats
 * 26-32: nada
 * 32: data
 *
 * always mono 8-bit alaw 8000 Hz. All the examples on the psion net site appear to be little endian.
 */

static void read_psion_header (int chan)
{
  chans = 1;
  header_distributed = 0;
  data_location = 32;
  srate = 8000;
  sound_format = snd_8_alaw;
  data_size = get_little_endian_int((unsigned char *)(hdrbuf+18)); /* always little-endian? */
  true_file_length=lseek(chan,0,SEEK_END);
  if ((true_file_length+32) != data_size) data_size = (true_file_length-32);
  data_size = c_snd_samples(sound_format,data_size);
}



/* ------------------------------------ Tandy DeskMate -------------------------------------
 * 
 * 0:  short 1A (c-Z = .snd ID byte)
 * 1:  data type (0=no compression, 1=music compression, 2=speech compression)
 * 2:  number of notes (= 1 if sound)
 * 3:  instrument number (= 0 if sound, otherwise this is an "instrument" file)
 * 4:  sound/instrument name (ASCIZ -- 10 bytes)
 * 14: sampling rate (apparently one of 5500, 11000, 22000 -- is this unsigned?)
 * 16: begin variable 28-byte records, after which is sound data as unsigned 8-bit linear ("PCM")
 *   in sound files:
 *   16: 0xFF 0 0xFF 0xFF
 *   20: data location if compressed
 *   24: 8 bytes of non-sound related things
 *   32: sound size
 *   36: 8 more useless bytes
 * 
 * always 8-bit unsigned linear mono
 * This info from oak.oakland.edu:/SimTel/msdos/sound/tspak17.zip (now at ftp.coast.net:/pub/SimTel/msdos/sound)
 */
static void read_deskmate_header (int chan)
{
  /* if we reach here, we're at 0 with HDRBUFSIZ read in */
  chans = 1;
  header_distributed = 0;
  data_location = 44;
  srate = get_little_endian_short((unsigned char *)(hdrbuf+14)); /* unsigned? */
  sound_format = snd_8_unsigned;
  data_size = get_little_endian_int((unsigned char *)(hdrbuf+32));
  true_file_length=lseek(chan,0,SEEK_END);
  data_size = c_snd_samples(sound_format,data_size);
}


/* ------------------------------------ Tandy DeskMate 2500 -------------------------------------
 * the "new .snd file format"
 *
 * 0:   name of sound, 10 byte ASCIZ
 * 10:  nada (probably to be backwards compatible...)
 * 44:  snd ID -- 0x1A80
 * 46:  number of samples
 * 48:  sound number
 * 50:  nada
 * 66:  compression code (see above)
 * 68:  nada
 * 88:  sampling rate
 * 90:  nada
 * 114: sample descriptors:
 *      0:  link to next (0 if last)
 *      4:  nada
 *      10: data location
 *      12: number of bytes in sample
 *      46: end of descriptor (presumably start of data)
 */
static void read_deskmate_2500_header (int chan)
{
  /* if we reach here, we're at 0 with HDRBUFSIZ read in */
  chans = 1;
  header_distributed = 0;
  data_location = 114+46;
  srate = get_little_endian_short((unsigned char *)(hdrbuf+88));
  sound_format = snd_8_unsigned;
  data_size = get_little_endian_int((unsigned char *)(hdrbuf+46));
  true_file_length=lseek(chan,0,SEEK_END);
  data_size = c_snd_samples(sound_format,data_size);
}


/* ------------------------------------ Gravis Ultrasound Patch -------------------------------------
 *
 * http://www.gravis.com/Public/sdk/PATCHKIT.ZIP
 *
 * header [128], instruments [62], layers [49], waveheaders (nested)
 * always little endian, actual files don't match exactly with any documentation
 *
 * Header block:
 *   0:  "GF1PATCH100" or "GF1PATCH110"
 *   12: "ID#000002"
 *   22: comment (copyright notice) (60 bytes ASCIZ)
 *   82: number of instruments
 *   83: number of voices
 *   84: wave channels
 *   85: number of waves
 *   87: vol
 *   89: size? 
 *   93: reserved (36? bytes)
 *
 * Instrument block:
 *   0: id
 *   2: name (16 bytes)
 *   18: size
 *   22: number of layers
 *   23: reserved (40? bytes)
 *
 * Layer block:
 *   0: "previous"
 *   1: id
 *   2: size
 *   6: number of wave samples
 *  10: reserved (40? bytes)
 *
 * Wave block:
 *   0: name (7 bytes ASCIZ)
 *   7: "fractions"
 *   8: data size of wave sample
 *  12: loop start
 *  16: loop end
 *  20: sample rate
 *  22: low freq
 *  26: high freq
 *  30: root freq
 *  34: tune
 *  36: balance
 *  37: envelope data (6+6 bytes I think)
 *  49: tremolo and vibrato data (6 bytes)
 *  55: mode bit 0: 8/16, 1: signed/unsigned
 *  56: scale freq
 *  58: scale factor
 *  60: reserved (36 bytes)
 *  followed by data presumably
 */

static void read_gravis_header(int chan)
{
  int mode;
  lseek(chan,0,SEEK_SET);
  read(chan,hdrbuf,128);
  chans = hdrbuf[84];
  if (chans == 0) chans=1;
  comment_start = 22;
  comment_end = 81;
  lseek(chan,239,SEEK_SET); /* try to jump to wave sample block (128+62+49) */
  read(chan,hdrbuf,128);
  srate = get_little_endian_unsigned_short((unsigned char *)(hdrbuf+20));
  data_size = get_little_endian_unsigned_short((unsigned char *)(hdrbuf+8));
  mode = hdrbuf[55];
  if (mode&1)
    {
      if (mode&2)
	sound_format = snd_16_unsigned_little_endian;
      else sound_format = snd_16_linear_little_endian;
    }
  else
    {
      if (mode&2)
	sound_format = snd_8_unsigned;
      else sound_format = snd_8_linear;
    }

  data_location = 337;
  header_distributed = 0;
  true_file_length = lseek(chan,0L,SEEK_END);
  data_size = c_snd_samples(sound_format,data_size);
}



/* ------------------------------------ Goldwave -------------------------------------
 *
 * http://web.cs.mun.ca/~chris3/goldwave/goldwave.html
 */

static void read_goldwave_header(int chan)
{
  chans = 1;
  header_distributed = 0;
  data_location = 28;
  data_size = get_little_endian_int((unsigned char *)(hdrbuf+22));
  true_file_length = lseek(chan,0L,SEEK_END);
  if (data_size <= 24) data_size = (true_file_length-data_location);
  srate = get_little_endian_int((unsigned char *)(hdrbuf+18));
  sound_format = snd_16_linear_little_endian;
}


/* ------------------------------------ Sonic Resource Foundry -------------------------------------
 *
 * more reverse engineering...
 * http://www.sfoundry.com/
 */

static void read_srfs_header(int chan)
{
  chans = 1; /* might be short at header[4] */
  header_distributed = 0;
  data_location = 32;
  true_file_length = lseek(chan,0L,SEEK_END);
  data_size = (true_file_length-data_location);
  srate = get_little_endian_int((unsigned char *)(hdrbuf+6));
  sound_format = snd_16_linear_little_endian;
}


/* ------------------------------------ Quicktime -------------------------------------
 *
 * infinitely complicated -- see Quicktime File Format doc from Apple.
 * there's no relation between this document and actual files -- a bizarre joke?
 */

static void read_qt_header(int chan)
{
  chans = 1;
  header_distributed = 0;
  data_location = 12;
  true_file_length = lseek(chan,0L,SEEK_END);
  data_size = (true_file_length-data_location);
  srate = 11025; /* ?? */
  sound_format = snd_8_unsigned;
}


/* ------------------------------------ SBStudioII -------------------------------------
 *
 * from a file created by Convert 1.4
 * 0: SND <space>
 * 8: file size - 8
 * SNNA SNIN SNDT blocks:
 *
 * built in blocks, other names are SNIN, SNDT
 * need to scan for SNDT, block length, data
 * SNNA len name 
 * supposedly ends with END (but my examples don't)
 * SNIN: 
 *   num (2), reserved (2), tuning (1), vol (2), type (2) bit 0: 1=PCM, bit 1: 1=16,0=8 (then loop data)
 * info from Pac.txt (pac.zip) at http://www.wotsit.demon.co.uk/music.htm 
 */

static void read_sbstudio_header(int chan)
{
  int i,happy,tmp;
  unsigned char *bp;
  lseek(chan,0,SEEK_SET);
  read(chan,hdrbuf,HDRBUFSIZ);
  chans = 1; 
  header_distributed = 0;
  srate = 8000; /* no sampling rate field in this header */
  sound_format = snd_16_linear_little_endian; /* might change later */
  true_file_length = lseek(chan,0L,SEEK_END);
  data_size = true_file_length - 56; /* first guess */
  happy = 1;
  i=8;
  bp = (unsigned char *)(hdrbuf+8);
  while (happy)
    {
      if (match_four_chars(bp,I_SNDT))
	{
	  data_size = get_little_endian_int((unsigned char *)(bp+4));
	  data_location = i+8;
	  happy = 0;
	}
      else
	{
	  if (match_four_chars(bp,I_SNIN))
	    {
	      tmp = get_little_endian_short((unsigned char *)(bp+15));
	      if ((tmp&1)==0) 
		sound_format = snd_unsupported;
	      else
		{
		  if ((tmp&2)==0) sound_format = snd_8_linear;
		}
	      i+=26;
	      bp+=26;
	    }
	  else
	    {
	      i++;
	      bp++;
	    }
	}
      if (i >= HDRBUFSIZ)
	{
	  sound_format = snd_unsupported;
	  happy = 0;
	}
    }
  data_size = c_snd_samples(sound_format,data_size);
}


/* ------------------------------------ Delusion Sound -------------------------------------
 *
 * more reverse engineering...
 * from a file created by Convert 1.4
 * 0: DDSF
 * 5: name (text)
 * 55: data
 * probaby similar to DMF format describe in Dmf-form.txt but I don't see any other block names in the data
 */

static void read_delusion_header(int chan)
{
  chans = 1; 
  header_distributed = 0;
  data_location = 55;
  true_file_length = lseek(chan,0L,SEEK_END);
  data_size = (true_file_length-data_location);
  srate = 8000;
  sound_format = snd_16_linear_little_endian;
  data_size = c_snd_samples(sound_format,data_size);
}



/* ------------------------------------ Farandole Composer WaveSample -------------------------------------
 *
 * 0: FSM 254
 * 4: name (text) (32 bytes)
 * 36: 10,13,26 or something like that
 * 39: len?
 * 40: volume
 * 41: looping data
 * 49: type (0=8-bit, else 16)
 * 50: loop mode
 * 51: data
 * described in Fsm.txt and Far-form.txt http://www.wotsit.demon.co.uk/music.htm 
 */

static void read_farandole_header(int chan)
{
  chans = 1; 
  header_distributed = 0;
  data_location = 51;
  true_file_length = lseek(chan,0L,SEEK_END);
  data_size = (true_file_length-data_location);
  srate = 8000;
  if (hdrbuf[49] == 0)
    sound_format = snd_8_linear;
  else sound_format = snd_16_linear_little_endian;
  data_size = c_snd_samples(sound_format,data_size);
}



/* ------------------------------------ Yamaha TX-16 -------------------------------------
 *
 * ftp://ftp.t0.or.at/pub/sound/tx16w/samples.yamaha
 * http://www.t0.or.at/~mpakesch/tx16w/
 *
 * from tx16w.c sox 12.15: (7-Oct-98) (Mark Lakata and Leigh Smith)
 *  char filetype[6] "LM8953"
 *  nulls[10],
 *  dummy_aeg[6]
 *  format 0x49 = looped, 0xC9 = non-looped
 *  sample_rate 1 = 33 kHz, 2 = 50 kHz, 3 = 16 kHz 
 *  atc_length[3] if sample rate 0, [2]&0xfe = 6: 33kHz, 0x10:50, 0xf6: 16, depending on [5] but to heck with it
 *  rpt_length[3] (these are for looped samples, attack and loop lengths)
 *  unused[2]
 */

static void read_tx16_header(int chan)
{
  chans = 1; 
  header_distributed = 0;
  data_location = 32;
  true_file_length = lseek(chan,0L,SEEK_END);
  data_size = (true_file_length-data_location);
  srate = 16000;
  if (hdrbuf[23] == 1) srate = 33000;
  else if (hdrbuf[23] == 2) srate = 50000;
  else if (hdrbuf[23] == 3) srate = 16000;
  else if (hdrbuf[23] == 0)
    {
      if ((hdrbuf[26]&0xFE) == 6) srate = 33000;
      else if ((hdrbuf[26]&0xFE) == 0x10) srate = 50000;
      else if ((hdrbuf[26]&0xFE) == 0xf6) srate = 16000;
    }
  original_sound_format = snd_12_linear_little_endian; /* can't read this format yet */
  sound_format = snd_unsupported;
  data_size = (int)((float)data_size / 1.5);
}


/* ------------------------------------ Yamaha SY-85 and SY-99 -------------------------------------
 *
 * more reverse engineering...
 * 0: SY85 (SY80 is SY-99) SY85ALL SY80 SYALL
 * 5: name ("WAVE1")
 * (26 int len)
 * (33: comment or prompt?)
 * data in 16-bit little endian (?)
 */

static void read_sy85_header(int chan)
{
  chans = 1; 
  header_distributed = 0;
  data_location = 1024;
  true_file_length = lseek(chan,0L,SEEK_END);
  data_size = (true_file_length-data_location);
  srate = 8000; /* unknown */
  sound_format = snd_16_linear; /* not right */
  data_size = c_snd_samples(sound_format,data_size);
}


/* ------------------------------------ Kurzweil 2000 -------------------------------------
 * 
 * "PRAM" then header len as big endian int??
 * from krz2tx.c (Mark Lakata)
 */
static void read_kurzweil_2000_header(int chan)
{
  chans = 1; 
  header_distributed = 0;
  data_location = get_big_endian_int((unsigned char *)(hdrbuf+4));
  true_file_length = lseek(chan,0L,SEEK_END);
  data_size = (true_file_length-data_location);
  srate = 44100; /* unknown */
  sound_format = snd_16_linear;
  data_size = c_snd_samples(sound_format,data_size);
}



/* ------------------------------------ Ultratracker WaveSample -------------------------------------
 *
 * 0..31: name (32=ctrl-Z?) 
 * 33: PMUWFD (but docs say this is "dos name" -- perhaps we can't recognize this header type reliably)
 * 44: 4 ints giving loop and size data
 * 60: vol
 * 61: "bidi" 0|8|24->8 bit else 16 -- but actual example has 0 with 16-bit
 * 62: finetune
 * 64: data (or 68?)
 * described in Ult-form.txt http://www.wotsit.demon.co.uk/music.htm 
 */

static void read_ultratracker_header(int chan)
{
  chans = 1; 
  header_distributed = 0;
  data_location = 64;
  true_file_length = lseek(chan,0L,SEEK_END);
  data_size = (true_file_length-data_location);
  srate = 8000;
  sound_format = snd_16_linear_little_endian;
  data_size = c_snd_samples(sound_format,data_size);
}



/* ------------------------------------ Sample dump exchange -------------------------------------
 *
 * 0: SDX:
 * sdx2tx.c (Mark Lakata) reads from 4 for 26 (^z), then
 * version (1)
 * comment as pascal-style string (byte len, bytes chars)
 * then 23 bytes:
 *  0: packing (0=pcm)
 *  1: midi channel
 *  2 + 256*[3]: sample number
 *  4: sample format (15: 16 bit unsigned(?), 8: 8bit unsigned(?)
 *  5: sample rate (big int?)
 *  9: sample length
 * 13: loop start
 * 17: loop end
 * 21: loop type 
 * 22: reserved
 */

static void read_sample_dump_header(int chan)
{
  int i,len;
  for (i=4;i<HDRBUFSIZ;i++) if (hdrbuf[i] == 26) break;
  len = hdrbuf[i+2];
  if (len>0)
    {
      comment_start = i+3;
      comment_end = i+3+len;
      }
  seek_and_read(chan, (unsigned char *)hdrbuf, i+3+len, HDRBUFSIZ);
  srate = get_little_endian_int((unsigned char *)(hdrbuf+5));
  /* data_size = get_little_endian_int((unsigned char *)(hdrbuf+9)); */
  if ((srate<100) || (srate>100000)) srate = 8000;
  chans = 1; 
  header_distributed = 0;
  data_location = i+3+len+23;;
  true_file_length = lseek(chan,0L,SEEK_END);
  data_size = (true_file_length-data_location);
  sound_format = snd_16_unsigned_little_endian;
  data_size = c_snd_samples(sound_format,data_size);
}


/* ------------------------------------ Digiplayer ST3 -------------------------------------
 *
 * 0: 1 (use 'SCRS' at 76)
 * 1: name
 * 13: nada
 * 14: "paragraph" offset of sample data
 * 16: length in bytes (looks like #samples in the actual files...)
 * 20: loop start
 * 24: loop end
 * 28: vol
 * 29: ?
 * 30: 0=unpacked, 1=DP30ADPCM
 * 31: bits: 0=loop, 1=stereo (chans not interleaved!), 2=16-bit samples (little endian)
 * 32: freq
 * 36: nada
 * 40: nada
 * 42: 512
 * 44: date?
 * 48: sample name (28 char ASCIZ)
 * 76: 'SCRS'
 * 80: data starts
 *
 * info from http://www.wotsit.demon.co.uk/music.htm (I forgot to write down the file name)
 */

static void read_digiplayer_header(int chan)
{
  chans = 1; 
  header_distributed = 0;
  data_location = 80;
  true_file_length = lseek(chan,0L,SEEK_END);
  data_size = (true_file_length-data_location);
  srate = 8000;
  sound_format = snd_16_unsigned_little_endian;
  if (hdrbuf[30]&2) chans=2;
  if (hdrbuf[30]&1) 
    sound_format = snd_unsupported;
  else
    {
      if (hdrbuf[30]&4) sound_format = snd_8_unsigned; /* may be backwards -- using Convert 1.4 output here */
    }
  data_size = c_snd_samples(sound_format,data_size);
}



/* ------------------------------------ CSRE adf -------------------------------------
 *
 * Info from Stuart Rosen
 *
 * 0-7: CSRE40
 * 8:   samples in file (long)
 * 12:  center line(?) (long)
 * 16:  start channel(?) (unsigned)
 * 18:  bits -- 12 or 16 (unsigned) -- is 12 bit sample file packed?
 * 20:  number system (0=signed, 1=unsigned)
 * 22:  srate in kHz (float)
 * 26:  peak sample in file (long) (can be 0)
 * 30-511: comment possibly
 *
 * probably always little-endian (S.R. reads each sample using sizeof(int) -> 16 bits I think)
 * if 12-bit unsigned we need to handle the offset somewhere
 */

static void read_adf_header(int chan)
{
  int bits,numsys;
  lseek(chan,0,SEEK_SET);
  read(chan,hdrbuf,30);
  chans = 1;
  numsys = get_little_endian_unsigned_short((unsigned char *)(hdrbuf+20));
  bits = get_little_endian_unsigned_short((unsigned char *)(hdrbuf+18));
  if ((bits == 16) || (bits == 12))
    {
      if (numsys == 0)
	sound_format = snd_16_linear_little_endian;
      else sound_format = snd_16_unsigned_little_endian;
    }
  else sound_format = snd_unsupported;
  srate = (int)get_little_endian_float((unsigned char *)(hdrbuf+22));
  data_size = get_little_endian_int((unsigned char *)(hdrbuf+8));
  data_location = 512;
  true_file_length = lseek(chan,0L,SEEK_END);
}



/* ------------------------------------ Diamondware -------------------------------------
 * 
 * info from Keith Weiner at DiamondWare (www.dw.com):
 *
 * 0-22:   DWD Header Byte "DiamondWare Digitized\n\0" 
 * 23:     1A (EOF to abort printing of file) 
 * 24:     Major version number 
 * 25:     Minor version number 
 * 26-29:  Unique sound ID (checksum XOR timestamp) 
 * 30:     Reserved 
 * 31:     Compression type (0=none) 
 * 32-33:  Sampling rate (in Hz) 
 * 34:     Number of channels (1=mono, 2=stereo) (interleaved)
 * 35:     Number of bits per sample (8, 16) (all data signed)
 * 36-37:  Absolute value of largest sample in file 
 * 38-41:  length of data section (in bytes) 
 * 42-45:  # samples (16-bit stereo is 4 bytes/sample) 
 * 46-49:  Offset of data from start of file (in bytes) 
 * 50-53:  Reserved for future expansion (markers) 
 * 54-55:  Padding 
 * 56:offset -- additional text: field=value
 *  suggested fields: TITLE, ORGARTIST, GENRE, KEYWORDS, ORGMEDIUM, EDITOR, DIGITIZER, COMMENT, SUBJECT, COPYRIGHT, SOFTWARE, CREATEDATE
 *
 * since this is all Windows/DOS oriented, I'll assume little-endian byte order.
 */

static void read_diamondware_header(int chan)
{
  lseek(chan,0,SEEK_SET);
  read(chan,hdrbuf,64);
  chans = hdrbuf[34];
  if (hdrbuf[31] == 0)
    {
      if (hdrbuf[35] == 8) sound_format = snd_8_linear;
      else sound_format = snd_16_linear_little_endian;
    }
  else sound_format = snd_unsupported;
  srate = get_little_endian_unsigned_short((unsigned char *)(hdrbuf+32));
  data_size = get_little_endian_int((unsigned char *)(hdrbuf+38));
  if (sound_format != snd_unsupported)
    data_size = c_snd_samples(sound_format,data_size);
  data_location = get_little_endian_int((unsigned char *)(hdrbuf+46));
  true_file_length = lseek(chan,0L,SEEK_END);
}


/* ------------------------------------ Comdisco SPW -------------------------------------
 * info from AFsp libtsp/AF/nucleus/AFgetSWpar.c
 *
 * header is text as in NIST:
 *
 *   $SIGNAL_FILE 9\n (12 chars)
 *   $USER_COMMENT
 *   <comment line(s)>
 *   $COMMON_INFO
 *   SPW Version        = 3.10
 *   System Type        = <machine> (e.g. "sun4","hp700")
 *   Sampling Frequency = <Sfreq>   (e.g. "8000")
 *   Starting Time      = 0
 *   $DATA_INFO
 *   Number of points   = <Nsamp>   (e.g. "2000")
 *   Signal Type        = <type>    ("Double", "Float", "Fixed-point", "Integer", "Logical")
 *   Fixed Point Format = <16,0,t> <16,16,t> <8,8,t> <8,0,t> (optional)
 *   Complex Format     = Real_Imag (optional)
 *   $DATA <data_type>              ("ASCII", "BINARY")
 *
 * the fixed point <n,m,b> is decoded as n=number of bits total per sample, m=integer bits, b=t: signed, u: unsigned
 * if $DATA ASCII, data is ascii text as in IEEE text files.
 * There are other complications as well.  We'll just hack up a stop-gap until someone complains.
 */

static void read_comdisco_header (int chan)
{
  /* need to grab a line at a time, call strcmp over and over.  This is very tedious. */
  char *line = NULL;
  char portion[32];
  char value[32];
  int i,j,k,m,n,curend,commenting,offset,len,little,type;
  int happy = 1;
  k = 15;
  line = (char *)CALLOC(256,sizeof(char));
  little = 0;
  offset = 0;
  type = 0;
  curend = INITIAL_READ_SIZE;
  commenting = 0;
  while (happy)
    {
      for (i=0;i<256;i++)
	{
	  if (k == curend)
	    {
	      offset += curend;
	      read(chan,hdrbuf,HDRBUFSIZ);
	      k = 0;
	      curend = HDRBUFSIZ;
	    }
	  if (hdrbuf[k] == '\n') {k++; break;}
	  line[i] = hdrbuf[k++];
	}
      line[i] = '\0';
      if ((strcmp(line,"$DATA BINARY") == 0) || (strcmp(line,"$DATA ASCII") == 0)) {happy = 0; data_location = offset+k;}
      if (strcmp(line,"$USER_COMMENT") == 0)
	{
	  comment_start = offset+k;
	  commenting = 1;
	}
      else
	{
	  if (commenting)
	    {
	      if (line[0] == '$')
		{
		  comment_end = offset+k-2-strlen(line);
		  commenting = 0;
		}
	    }
	}
      if (line[0] != '$')
	{
	  len = strlen(line);
	  for (j=0;j<8;j++) portion[j]=line[j];
	  portion[8]='\0';
	  for (j=8;j<len;j++) if (line[j] == '=') break;
	  for (n=0,m=j+2;m<len;m++,n++) value[n]=line[m];
	  value[n]='\0';
	  if (strcmp(portion,"Sampling") == 0) sscanf(value,"%d",&srate); else
	  if (strcmp(portion,"Number o") == 0) sscanf(value,"%d",&data_size); else
	  if (strcmp(portion,"Signal T") == 0) {if (value[1] == 'o') type=2; else if (value[1] == 'l') type=1;} else
	  if (strcmp(portion,"Fixed Po") == 0) {if (value[1] == '8') type=3;}
	}
    }
  /* now clean up this mess */
  chans = 1;
  header_distributed = 0;
  switch (type)
    {
    case 0: if (little) sound_format = snd_16_linear_little_endian; else sound_format = snd_16_linear; break;
    case 1: if (little) sound_format = snd_32_float_little_endian; else sound_format = snd_32_float; break;
    case 2: if (little) sound_format = snd_64_double_little_endian; else sound_format = snd_64_double; break;
    case 3: sound_format = snd_8_linear; break;
    }
  true_file_length = lseek(chan,0L,SEEK_END);
  FREE(line);
}


/* ------------------------------------ MS ASF -------------------------------------
 *
 * asf format is described at http://www.microsoft.com/asf/specs.htm
 * http://www.microsoft.com/asf/spec3/ASF0198ps.exe
 *
 * this header is completely insane
 */

static void read_asf_header (int chan)
{
  /* a chunked data format, so not really acceptable here or elsewhere -- needs to be unchunked */
  int len,ilen = 0,i,j,asf_huge,present,bits = 0;
  /* apparently "huge" has some meaning in Windoze C */
  len = get_little_endian_int((unsigned char *)(hdrbuf+16)); /* actually 64 bits */
  i = (128+64) / 8;
  asf_huge = 0;
  srate = 0;
  chans = 0;
  while (i<len)
    {
      seek_and_read(chan,(unsigned char *)hdrbuf,i,HDRBUFSIZ);
      if ((unsigned int)(hdrbuf[1]) == 0x29) 
	switch (hdrbuf[0])
	  {
	  case 0xd0: 
	    asf_huge = (hdrbuf[((128+64+128+64+64+64+64+32)/8)] & 2);
	    break;
	  case 0xd4: 
	    present = ((hdrbuf[16+8+16+8+8+ 4+4+4+4+ 4+4] >> 3) & 0x3);
	    if (present)
	      j = 16+8+16+8+8+ 4+4+4+4+ 4+4+ 4+ (4+4+4) + 2;
	    else j = 16+8+16+8+8+ 4+4+4+4+ 4+4+ 4+ 2;
	    srate = get_little_endian_int((unsigned char *)(hdrbuf+j+11+36));
	    bits = get_little_endian_int((unsigned char *)(hdrbuf+j+11+32));
	    chans = get_little_endian_unsigned_short((unsigned char *)(hdrbuf+j+65));
	    original_sound_format = get_little_endian_int((unsigned char *)(hdrbuf+j+11));
	    break;
	  default: break;
	  }
      ilen = get_little_endian_int((unsigned char *)(hdrbuf+16));
      if (ilen <= 0) break;
      if ((chans>0) && (srate>0)) break;
      i+=ilen;
    }
  i=len;
  seek_and_read(chan,(unsigned char *)hdrbuf,i,HDRBUFSIZ);
  sound_format = snd_unsupported;
  if (((unsigned int)(hdrbuf[1]) == 0x29) && ((unsigned int)(hdrbuf[0]) == 0xd2))
    {
      ilen = get_little_endian_int((unsigned char *)(hdrbuf+16));
      if (asf_huge) asf_huge=4; else asf_huge=2;
      data_location = i+20+asf_huge+2+4+3+1;
      if (bits == 0) bits=8;
      sound_format = wave_to_sndlib_format(original_sound_format,bits,1);
    }
  if (sound_format != snd_unsupported)
    data_size = c_snd_samples(sound_format,(ilen - data_location));
}



/* ------------------------------------ no header, Sound Designer II, SoundEdit, SoundEdit 16 ------------------------------------- 
 *
 * Sound Designer II data fork is a raw data file -- interleaved channels, bytes in sequence.
 *   The associated resource fork under "STR " has "sample-size" "sample-rate" and "channels".
 *   Similarly the resources "sdDD" id 1000, "sdML" id 1000, and "sdLL" id 1000 return pascal records of
 *   respectively Document Data, Markers and text, Loop data.  I don't see any useful info in any of these
 *   except perhaps the file comment in the "sdDD" record of type str255 10 bytes in (I think).
 *   See headers.lisp for a reader for some of this stuff.
 *   file type: 'Sd2f'
 *
 * SoundEdit data fork contains 8-bit unsigned linear samples.  The resource fork has:
 *   REPT: 0: mouse pos
 *         4: length of sound in samples
 *         8: sample number of beginning of selection (if any)
 *        12: ditto end of selection
 *        16: ditto loopback beginning (if any)
 *        20: ditto loopback end
 *   INFO: 0: nada
 *         8: "record frequency"
 *        12: ditto for playback
 *        16: compression mode: 0=none
 *        20: chans (0=mono, 1=stereo)
 *        24: window position
 *        28: sample rate: 1=22KHz, 2=11, 3=7, 4=5
 *        32: length of left channel in samples
 *        36: if stereo, ditto right channel
 *   file type: 'FSDD', creator: 'SFX!'
 * 
 * SoundEdit 16 data fork has either 8 or 16-bit linear unsigned samples.  The resource fork has:
 *   TRKS: 0: -7 (version number)
 *         4: track length in bytes
 *         8: track offset in samples
 *        12: left/right stereo
 *        16: track selected flag
 *        20: sample rate
 *        22: pstring -- track title (31 chars, fixed size)
 *        54: (?) gain
 *      one TRKS resource for each track in the file
 *   INFO: essentially the same as above, but with added version number at 
 *        40: 0=SoundEdit, -2=SoundEdit 16
 *        44: reserved
 *        52: sample rate
 *        and then other stuff that doesn't look interesting
 *   LABS: 16: pstring (31 chars) track label
 *   CUES: 0: sample location
 *         4: pstring (31 chars) cue point text
 *   PReS: spectral stuff
 *   PRnt: printer stuff
 *   CLRS: colors stuff
 *   REPT: same as above
 *   file type: 'jB1 ', creator: 'jBox'.
 */

static void read_no_header (int chan)
{
  srate = 44100;
  chans = 2;
  header_distributed = 0;
  data_location = 0;
  data_size = lseek(chan,0L,SEEK_END);
  true_file_length = data_size;
  sound_format = snd_16_linear;
  data_size = c_snd_samples(sound_format,data_size);
  /* read-header in headers.lisp can ask the user if this is really what he wants */
  /* The upshot is that fasmix (i.e. merge) cannot deal directly with raw sound data */
}


#ifdef MACOS

/* try to read SDII resources */
#include <Resources.h>
#include <Sound.h>

static int get_resource(char *resname)
{
  Handle sndH;
  OSErr err;
  char *val;
  int rtn = -1;
  sndH = GetNamedResource('STR ',(const unsigned char *)c2pstr(resname));
  err = ResError();
  if ((err == noErr) && (sndH))
    {
      LoadResource(sndH);
      val = p2cstr((unsigned char *)(*sndH));
      sscanf(val,"%d",&rtn);
      ReleaseResource(sndH);
    }
  return(rtn);
}

#endif


/* ------------------------------------ all together now ------------------------------------ */

void c_read_header_with_fd (int chan) /* internal optimization for merge.c */
{
  int i,happy,loc = 0;
  read(chan,hdrbuf,INITIAL_READ_SIZE);
  header_type = unsupported_sound_file;
  sound_format = snd_unsupported;
  comment_start = 0;
  comment_end = 0;
  if ((match_four_chars((unsigned char *)hdrbuf,I_DSND)) || (match_four_chars((unsigned char *)hdrbuf,I_DECN)))
    {
      header_type = NeXT_sound_file;
      read_next_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_FORM))
    {
      /* next 4 bytes are apparently the file size or something equally useless */
      if ((match_four_chars((unsigned char *)(hdrbuf+8),I_AIFF)) || (match_four_chars((unsigned char *)(hdrbuf+8),I_AIFC)))
	{ 
	  header_type = AIFF_sound_file;
	  read_aiff_header(chan,0);
	  return;
	}
      if (match_four_chars((unsigned char *)(hdrbuf+8),I_8SVX))
	{
	  header_type = SVX_sound_file;
	  read_8svx_header(chan);
	  return;
	}
      if (match_four_chars((unsigned char *)(hdrbuf+8),I_MAUD))
	{
	  header_type = MAUD_sound_file;
	  read_maud_header(chan);
	}
      return;
    }
  if ((match_four_chars((unsigned char *)hdrbuf,I_RIFF)) || match_four_chars((unsigned char *)hdrbuf,I_RIFX))
    {
      if (match_four_chars((unsigned char *)(hdrbuf+8),I_WAVE))
	{
	  header_type = RIFF_sound_file;
	  read_riff_header(chan);
	  return;
	}
      if (match_four_chars((unsigned char *)(hdrbuf+8),I_sfbk))
	{
	  header_type = soundfont_sound_file;
	  read_soundfont_header(chan);
	  return;
	}
      if (match_four_chars((unsigned char *)(hdrbuf+8),I_AVI_))
	{
	  header_type = AVI_sound_file;
	  read_avi_header(chan);
	}
      return;
    }
  if ((equal_big_or_little_endian((unsigned char *)hdrbuf,I_IRCAM_VAX)) || 
      (equal_big_or_little_endian((unsigned char *)hdrbuf,I_IRCAM_SUN)) ||
      (equal_big_or_little_endian((unsigned char *)hdrbuf,I_IRCAM_MIPS)) || 
      (equal_big_or_little_endian((unsigned char *)hdrbuf,I_IRCAM_NEXT)))
    {
      header_type = IRCAM_sound_file;
      read_ircam_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_NIST))
    {
      header_type = NIST_sound_file;
      read_nist_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_SOUN))
    {
      if ((match_four_chars((unsigned char *)(hdrbuf+4),I_SMP1)) && (match_four_chars((unsigned char *)(hdrbuf+8),I_SMP2)))
	{
	  header_type = SMP_sound_file;
	  read_smp_header(chan);
	}
      else
	{
	  header_type = SNDT_sound_file;
	  read_sndt_header(chan);
	}
      return;
    }
  if ((match_four_chars((unsigned char *)hdrbuf,I_VOC0)) && (match_four_chars((unsigned char *)(hdrbuf+4),I_VOC1)))
    {
      header_type = VOC_sound_file;
      read_voc_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_AVR_))
    {
      header_type = AVR_sound_file;
      read_avr_header(chan);
      return;
    }
  if (get_big_endian_short((unsigned char *)hdrbuf) == 1336)
    {
      header_type = SD1_sound_file;
      read_sd1_header(chan);
      return;
    }
  if ((match_four_chars((unsigned char *)hdrbuf,I_ALaw)) && (match_four_chars((unsigned char *)(hdrbuf+4),I_Soun)))
    {
      header_type = PSION_sound_file;
      read_psion_header(chan);
      return;
    }
  if ((match_four_chars((unsigned char *)hdrbuf,I_GF1P)) && (match_four_chars((unsigned char *)(hdrbuf+4),I_ATCH)))
    {
      header_type = gravis_sound_file;
      read_gravis_header(chan);
      return;
    }
  if ((match_four_chars((unsigned char *)hdrbuf,I_DSIG)) && (match_four_chars((unsigned char *)(hdrbuf+4),I_NAL_)))
    {
      header_type = comdisco_sound_file;
      read_comdisco_header(chan);
      return;
    }
  if ((match_four_chars((unsigned char *)hdrbuf,I_GOLD)) && (match_four_chars((unsigned char *)(hdrbuf+4),I__WAV)))
    {
      header_type = goldwave_sound_file;
      read_goldwave_header(chan);
      return;
    }
  if ((match_four_chars((unsigned char *)hdrbuf,I_Diam)) && (match_four_chars((unsigned char *)(hdrbuf+4),I_ondW)))
    {
      header_type = DiamondWare_sound_file;
      read_diamondware_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_SRFS))
    {
      header_type = srfs_sound_file;
      read_srfs_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_CSRE))
    {
      header_type = ADF_sound_file;
      read_adf_header(chan);
      return;
    }
  if ((hdrbuf[0] == 0xf0) && (hdrbuf[1] == 0x7e) && (hdrbuf[3] == 0x01))
    {
      header_type = MIDI_sample_dump;
      chans = 1;
      srate = srate = (int)(1.0e9 / (float)((hdrbuf[7] + (hdrbuf[8]<<7) + (hdrbuf[9]<<14))));
      data_size = (hdrbuf[10] + (hdrbuf[11]<<7) + (hdrbuf[12]<<14));
      /* since this file type has embedded blocks, we have to translate it elsewhere */
      return;
    }
  /* no recognized magic number at start -- poke around in possible header for other types */
  /* ESPS is either 0x00006a1a or 0x1a6a0000 at byte 16 */
  if (equal_big_or_little_endian((unsigned char *)(hdrbuf+16),0x00006a1a))
    {
      header_type = ESPS_sound_file;
      read_esps_header(chan);
      return;
    }
  lseek(chan,0,SEEK_SET);
  read(chan,hdrbuf,256);
  if ((hdrbuf[252]==64) && (hdrbuf[253]==195)) /* #o100 and #o303 */
    {
      header_type = SPPACK_sound_file;
      read_sppack_header(chan);
      return;
    }
  if ((match_four_chars((unsigned char *)(hdrbuf+65),I_FSSD)) && (match_four_chars((unsigned char *)(hdrbuf+128),I_HCOM)))
    {
      header_type = HCOM_sound_file;
      return;
    }
  happy = 0;
  for (i=0;i<NINRS;i++) 
    {
      if (equal_big_or_little_endian((unsigned char *)hdrbuf,I_INRS[i]))
	{
	  happy = 1;
	  loc = inrs_srates[i];
	}
    }
  if (happy)
    {
      header_type = INRS_sound_file;
      read_inrs_header(chan,loc);
      return;
    }
  if (get_big_endian_unsigned_int((unsigned char *)hdrbuf) == 0xAAAAAAAA)
    {
      header_type = MUS10_sound_file;
      return;
    }
  if ((match_four_chars((unsigned char *)hdrbuf,I_SPIB)) || (match_four_chars((unsigned char *)hdrbuf,I_S___)))
    {
      header_type = IEEE_sound_file;
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_DVSM))
    {
      header_type=DVSM_sound_file;
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_Drat))
    {
      header_type=RealAudio_sound_file;
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_MThd))
    {
      header_type=MIDI_file;
      return;
    }
  if ((match_four_chars((unsigned char *)hdrbuf,I_Esig)) && (match_four_chars((unsigned char *)(hdrbuf+4),I_nalc)))
    {
      header_type=Esignal_file;
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_SND_))
    {
      header_type = SBStudioII_sound_file;
      read_sbstudio_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_FSMt))
    {
      header_type = Farandole_sound_file;
      read_farandole_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_SDXc))
    {
      header_type = Sample_dump_sound_file;
      read_sample_dump_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_DDSF))
    {
      header_type = Delusion_sound_file;
      read_delusion_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_LM89))
    {
      header_type = Yamaha_TX16_sound_file;
      read_tx16_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_SY85))
    {
      header_type = Yamaha_SY85_sound_file;
      read_sy85_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_SY80))
    {
      header_type = Yamaha_SY99_sound_file;
      read_sy85_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_PRAM))
    {
      header_type = Kurzweil_2000_sound_file;
      read_kurzweil_2000_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)(hdrbuf+35),I_UWFD))
    {
      header_type = Ultratracker_sound_file;
      read_ultratracker_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)(hdrbuf+76),I_SCRS))
    {
      header_type = digiplayer_sound_file;
      read_digiplayer_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_covox))
    {
      header_type = Covox_sound_file;
      read_covox_header(chan);
      return;
    }
  if (match_four_chars((unsigned char *)hdrbuf,I_DSPL))
    {
      header_type = SPL_sound_file;
      read_spl_header(chan);
      return;
    }
  if (get_big_endian_short((unsigned char *)hdrbuf) == 0x1A00)
    {
      header_type = DeskMate_sound_file;
      read_deskmate_header(chan);
      return;
    }
  if (get_big_endian_short((unsigned char *)(hdrbuf+44)) == 0x1A80)
    {
      header_type = DeskMate_2500_sound_file;
      read_deskmate_2500_header(chan);
      return;
    }
#ifdef SNDLIB_LITTLE_ENDIAN
  if (get_uninterpreted_int((unsigned char *)hdrbuf) == 0x01000800)
#else
  if (get_uninterpreted_int((unsigned char *)hdrbuf) == 0x00080001)
#endif
    {
      header_type = ADC_sound_file;
      read_adc_header(chan);
      return;
    }

  if ((match_four_chars((unsigned char *)hdrbuf,I_ones)) &&
      (match_four_chars((unsigned char *)(hdrbuf+12),I_FORM)))
    {
      /* possibly an OMF file with an embedded AIFF data file -- this is just a guess... */
      header_type = OMF_sound_file;
      read_aiff_header(chan,12);
      return;
    }

  if ((match_four_chars((unsigned char *)hdrbuf,I_zeros)) &&
      (match_four_chars((unsigned char *)(hdrbuf+4),I_mdat)))
    {
      /* possibly quicktime?? */
      header_type = Quicktime_sound_file;
      read_qt_header(chan);
      return;
    }
  
  if ((match_four_chars((unsigned char *)hdrbuf,I_asf0)) &&
      (match_four_chars((unsigned char *)(hdrbuf+4),I_asf1)) &&
      (match_four_chars((unsigned char *)(hdrbuf+8),I_asf2)) &&
      (match_four_chars((unsigned char *)(hdrbuf+12),I_asf3)))
    {
      header_type = asf_sound_file;
      read_asf_header(chan);
      return;
    }

  header_type = raw_sound_file;
  read_no_header(chan);
}

int c_read_header (char *name)
{
  int chan;
#ifdef MACOS
  int loc,happy;
#endif
  chan = clm_open_read(name);
  if (chan == -1) return(-1);
  c_read_header_with_fd(chan);

#ifdef MACOS
  /* on the Mac, we should look for the resource fork. */
  /* we'll at least check SDII possibilities */
  /* if I had an example of SoundEdit output, I'd add support for that too */
  if (header_type == raw_sound_file)
    {
      loc = OpenResFile((const unsigned char *)c2pstr(name));
      if (loc != -1)
	{
	  happy = get_resource("channels");
	  if (happy != -1)
	    {
	      /* found an SDII file?!? */
	      chans = happy;
	      srate = get_resource("sample-rate");
	      happy = get_resource("sample-size");
	      if (happy == 1)
		sound_format = snd_8_unsigned;
	      else sound_format = snd_16_linear;
	      data_size = c_snd_samples(sound_format,true_file_length);
	      header_type = SD2_sound_file;
	    }
	  CloseResFile(loc);
	}
    }
  /* we could also check the file type:
   * FInfo fi;
   * err = GetFInfo((const unsigned char *)c2pstr(name),fd,&fi);
   * if (err == noErr) <check fi.fdType against possibiliites given above>
   */

#endif

  close(chan);  
  return(0);
}

int c_write_header_with_fd (int chan, int type, int in_srate, int in_chans, int loc, int size, int format, char *comment, int len)
{
  int siz;
  siz = c_snd_bytes(format,size);
  switch (type)
    {
    case NeXT_sound_file: write_next_header(chan,in_srate,in_chans,loc,siz,format,comment,len); break;
    case AIFF_sound_file: write_aiff_header(chan,in_srate,in_chans,siz,format,comment,len); break;
    case RIFF_sound_file: write_riff_header(chan,in_srate,in_chans,siz,format,comment,len); break;
    case IRCAM_sound_file: write_ircam_header(chan,in_srate,in_chans,format,comment,len); break;
    case NIST_sound_file: write_nist_header(chan,in_srate,in_chans,siz,format); break;
    case raw_sound_file: 
      data_location = 0; 
      data_size = c_snd_samples(format,siz);
      srate = in_srate; 
      chans = in_chans; 
      header_type = raw_sound_file;
      sound_format = format;
      break;
    default:
      {
	return(-1);
	break;
      }
    }
  return(0);
}

int c_write_header (char *name, int type, int in_srate, int in_chans, int loc, int size, int format, char *comment, int len)
{
  int chan,err;
  chan = clm_create(name);
  if (chan == -1) return(-1);
  err = c_write_header_with_fd(chan,type,in_srate,in_chans,loc,size,format,comment,len);
  close(chan);
  return(err);
}

int c_update_header_with_fd(int chan, int type, int siz)
{
  /* do not export unless you remember that siz here is in bytes! */
  lseek(chan,0L,SEEK_SET);
  switch (type)
    {
    case NeXT_sound_file: update_next_header(chan,siz); break;
    case AIFF_sound_file: update_aiff_header(chan,siz); break;
    case RIFF_sound_file: update_riff_header(chan,siz); break;
    case IRCAM_sound_file: update_ircam_header(); break;
    case NIST_sound_file: update_nist_header(chan,siz); break;
    case raw_sound_file: break;
    default:
      {
	return(-1);
	break;
      }
    }
  return(0);
}

int c_update_header (char *name, int type, int size, int srate, int format, int chans, int loc)
{
  int chan,siz,err;
  chan = clm_reopen_write(name);
  if (chan == -1) return(-1);
  siz = c_snd_bytes(format,size);
  err = c_update_header_with_fd(chan,type,siz);
  if (type == NeXT_sound_file)
    {
      if (srate != 0)
	{
	  lseek(chan,16L,SEEK_SET);
	  set_big_endian_int((unsigned char *)hdrbuf,srate);
	  write(chan,hdrbuf,4);
	}
      if (chans != 0)
	{
	  lseek(chan,20L,SEEK_SET);
	  set_big_endian_int((unsigned char *)hdrbuf,chans);
	  write(chan,hdrbuf,4);
	}
      if (loc != 0)
	{
	  lseek(chan,4L,SEEK_SET);
	  set_big_endian_int((unsigned char *)hdrbuf,loc);
	  write(chan,hdrbuf,4);
	}
    }
  close(chan);
  return(err);
}

#ifdef CLM
int excl_c_update_header (char *name, int type, int *siz, int srate, int format, int chans, int loc)
{
  return(c_update_header(name,type,siz[0]+(siz[1]<<16),srate,format,chans,loc));
}

int excl_c_write_header (char *name, int type, int srate, int chans, int loc, int *siz, int format, char *comment, int len)
{
  return(c_write_header(name,type,srate,chans,loc,(siz[0]+(siz[1]<<16)),format,comment,len));
}
#endif

int c_update_header_comment (char *name, int loc, char *comment, int len, int typ)
{
  int chan;
  chan = clm_reopen_write(name);
  if (chan == -1) return(-1);
  switch (typ)
    {
    case NeXT_sound_file: update_next_header_comment(chan,loc,comment,len); break;
    case AIFF_sound_file: update_aiff_header_comment(chan,comment,len); break;
    case RIFF_sound_file: update_riff_header_comment(chan,comment,len); break;
    case raw_sound_file: break;
    default:
      {
	return(-1);
	break;
      }
    }
  return(0);
}



/* ------------------------------------------------------------------------
 * old Mus10, SAM formats, just for completeness
 *
 * These were used for sound data on the PDP-10s at SAIL and CCRMA in the 70's and 80's.
 * The word length was 36-bits. (SAM here refers to the Samson box, the Systems Concepts
 * Digital Synthesizer, built by Peter Samson), not the SAM format associated (apparently)
 * with ModEdit.
 *
 * "New" format as used by nearly all CCRMA software pre-1990:
 *
 *  WD 0 - '525252525252 (chosen because "it is unlikely to occur at random"!)
 *  WD 1 - Clock rate in Hz (PDP-10 36-bit floating point)
 *         PDP-10 floating point format was sign in bit 0, excess 128 exponent in 1-8, fraction in 9-35
 *  WD 2 - #samples per word,,pack-code, (has # samples per word in LH, pack-code in RH)
 * 	0 for 12-bit fixed point (this was the main one)
 * 	1 for 18-bit fixed point
 * 	2 for  9-bit floating point incremental (never used)
 * 	3 for 36-bit floating point (never used)
 * 	4 for 16-bit sambox fixed point, right justified
 * 	5 for 20-bit sambox fixed point
 * 	6 for 20-bit right-adjusted fixed point (sambox SAT format=SAM output)
 * 	7 for 16-bit fixed point, left justified (never used)
 * 	N>9 for N bit bytes in ILDB format (never used)
 *  WD 3 - # channels
 *  WD 4 - Maximum amplitude (if known) (PDP-10 float)
 *  WD 5 - number of Sambox ticks per pass (inverse of Sambox clock rate, sort of)
 *  WD 6 - Total #samples in file. If 0 then #wds_in_file*#samps_per_wd assumed.
 *  WD 7 - Block size (if any). 0 means sound is not blocked.
 *  WDs '10-'77 Reserved for EDSND usage, but I used it anyway.
 *  WDs '100-'177 Text description of file (in ASCIZ format) (ASCIZ=ASCII+null if I remember right = C string(?))
 *
 * "Old" format (pre-1977)
 *
 *  WD 0 - '525252525252
 *  WD 1 - Clock rate => rate as integer,,code
 * 	code=0 for 6.4Kc (or anything else)
 * 	    =1 for 12.8Kc, =2 for 25.6Kc, =3 for 51.2Kc
 * 	    =5 for 102.4Kc, =6 for 204.8Kc
 *  WD 2 - #sample per word,,pack
 * 	0 for 12 bit
 * 	1 for 16 bit (18 bit)
 * 	2 for 9 bit floating point incremental
 * 	3 for 36-bit floating point
 * 	N>9 for N bit bytes in ILDB format
 *  WD 3 - # channels
 *  WD 4 - Maximum amplitude (if known, otherwise 0)
 *  WDs 5-77 Reserved for future expansion
 *  WDs 100-177 Text description of file (in ASCIZ format) 
 *
 * ------------------------------------
 * and even more esoteric... from the MIT PDP-6 (1971-76):
 *      JRST 1
 *      0
 *      blkcnt,,2000
 *	duration,, ---    ; negative halfword number to be counted down for duration
 *	v0 v1 v2 v3 v4 v5 ; 6 six-bit byes, each encoding pitch no. from 2.-63, 63 not used
 *	duration,, --
 *	v0 v1 v2 v3 v4 v5 
 *	...
 *	checksum
 *      ...
 *	blkcnt,,blkaddr   ; neg half word in block, not counting itself or checksum,,addr of block start
 *	...
 *	0,,--
 *	0
 *      checksum          ;=ROT 1 and ADD including blkcnt,,blkaddr
 *	-1,,41
 *	JRST 101
 *	checksum
 *	-1,,41
 *	JRST 101
 *	checksum
 *	JUMPA 101
 *	JUMPA 101
 *
 * ah, the good old days...
 */

#if 0
One supported sample type is the DVSM-Format of the programs WinRec, WinCut
and Fortune. Those are programs for the ATARI FALCON.

A DVSM sample file has the following structure:

	char magic[6];     /* "DVSM" */
	int headlen;       /* Headlen in Bytes*/
	int freq;	   /* Sample freqency 0=8kHz 7=50kHz*/
	char pack;	   /* 0 unpacked, 2=DVS packmethod*/
	char mode;         /* 0=Stereo 8Bit,1=Stereo 16Bit,2=Mono 8Bit,3=Mono 16*/
	long blocklen;     /* if pack>0: Length of a packed block*/ 

followed by cookies and the sound data.

The sample frequencies 0 to 7 correspond to the following frequencies:
sam_freq[8]={8195,9834,12292,16390,19668,24585,32778,49170};
For further information refer to the WinRec documentation or take a look at
the article series 'Sound Sample Formate' in the german magazin ST Computer
(3/94 - 6/94).

Confusingly enough there also appears to be a DVMS header (see sox11 cvsd.c)
struct dvms_header {
	char          Filename[14];
	unsigned      Id;
	unsigned      State;
	time_t        Unixtime;
	unsigned      Usender;
	unsigned      Ureceiver;
	ULONG	      Length;
	unsigned      Srate;
	unsigned      Days;
	unsigned      Custom1;
	unsigned      Custom2;
	char          Info[16];
	char          extend[64];
	unsigned      Crc;
};
#define DVMS_HEADER_LEN 120

#endif

#if 0
  /*
SCRIBE is headerless, 16-bit little endian data

(Hidden Markov Model ToolKit output -- obscure!)
HTK format files consist of a contiguous sequence of samples preceded by a header. Each sample is a vector of either 2-byte
integers or 4-byte floats. 2-byte integers are used for compressed forms as described below and for vector quantised data as
described later in section 5.11. HTK format data files can also be used to store speech waveforms as described in section 5.8.   

The HTK file format header is 12 bytes long and contains the following data 
  nSamples   -- number of samples in file (4-byte integer)
  sampPeriod -- sample period in 100ns units (4-byte integer)
  sampSize   -- number of bytes per sample (2-byte integer)
  parmKind   -- a code indicating the sample kind (2-byte integer)

The parameter kind  consists of a 6 bit code representing the basic parameter kind plus additional bits for each of the possible
qualifiers . The basic parameter kind codes are 
 
 0    WAVEFORM    sampled waveform
 1    LPC         linear prediction filter coefficients
 2    LPREFC      linear prediction reflection coefficients
 3    LPCEPSTRA   LPC cepstral coefficients
 4    LPDELCEP    LPC cepstra plus delta coefficients
 5    IREFC       LPC reflection coef in 16 bit integer format
 6    MFCC        mel-frequency cepstral coefficients
 7    FBANK       log mel-filter bank channel outputs
 8    MELSPEC     linear mel-filter bank channel outputs
 9    USER        user defined sample kind
 10   DISCRETE    vector quantised data

and the bit-encoding for the qualifiers (in octal) is 
  _E   000100      has energy
  _N   000200      absolute energy suppressed
  _D   000400      has delta coefficients
  _A   001000      has acceleration coefficients
  _C   002000      is compressed
  _Z   004000      has zero mean static coef.
  _K   010000      has CRC checksum
  _O   020000      has 0'th cepstral coef.
*/
#endif

#if 0
/*
Esignal starts with 48 byte ASCII preamble:

"Esignal"\n
7 byte version id\n
7 byte left padded data format choice (i.e. "   EDR1", EDR2, ASCII, or machine name) EDR1=4byte ints, EDR2=8 byte longs\n
7 byte preamble size (i.e. "     48")\n
7 byte left padded header size (bytes, includes preamble)\n
7 byte left padded record size\n

for example:
Esignal
   0.0B
  ASCII
     48
    502
     -1

default formats are big endian ints and shorts, IEEE floats

array desc:
short data type code
short rank (i.e. nchans in this context I guess)
long dimensions (i.e. sample number? -- rank of these)
<data>

type codes:
1 NO_TYPE 10 UCHAR
2 ARRAY   11 BOOL
3 DOUBLE  12 DOUBLE_COMPLEX
4 FLOAT   13 FLOAT_COMPLEX
5 LONG    14 LONG_COMPLEX
6 ULONG   15 SHORT_COMPLEX
7 SHORT   16 SCHAR_COMPLEX
8 USHORT  17 CHAR
9 SCHAR   18 WCHAR

here's a simple example:

                        PREAMBLE
0    45 73 69 67 6e 61 6c 0a     "Esignal\n"       "magic number"
8    20 20 20 30 2e 30 42 0a     "   0.0B\n"       version
16   20 20 20 45 44 52 31 0a     "   EDR1\n"       architecture
24   20 20 20 20 20 34 38 0a     "     48\n"       preamble size
32   20 20 20 20 32 38 35 0a     "    285\n"       header size
40   20 20 20 20 20 20 32 0a     "      2\n"       record size
                        START OF FIELD LIST
48   00 00 00 04                 4                 number of fields
                        FIRST FIELD: commandLine
52   00 00 00 0b                 11                length of name
56   63 6f 6d 6d 61 6e 64 4c     "commandL"        field name
64   69 6e 65                    "ine"             ...
67   00 11                       17 (CHAR)         data type
69   00 01                       1                 number of dimensions
71   00 00 00 11                 17                dimension
75   00 00 00 00                 0                 length of units
79   3f f0 00 00 00 00 00 00     1.0               scale
87   00 00 00 00 00 00 00 00     0.0               offset
95   00 00 00 00                 0                 number of axis names
99   00 01                       1 (GLOBAL)        occurrence class
101  45 72 65 63 6f 72 64 20     "Erecord "        data
109  73 70 65 65 63 68 2e 73     "speech.s"        ...
117  64                          "d"               ...
118  00 00 00 00                 0                 number of subfields
                        SECOND FIELD: startTime
122  00 00 00 09                 9                 length of name
126  73 74 61 72 74 54 69 6d     "startTim"        field name
134  65                          "e"               ...
135  00 03                       3 (DOUBLE)        data type
137  00 00                       0                 number of dimensions
139  00 00 00 01                 1                 length of units
143  73                          "s"               units
144  3f f0 00 00 00 00 00 00     1.0               scale
152  00 00 00 00 00 00 00 00     0.0               offset
160  00 00 00 00                 0                 number of axis names
164  00 01                       1 (GLOBAL)        occurrence class
166  00 00 00 00 00 00 00 00     0.0               data
174  00 00 00 00                 0                 number of subfields
                       THIRD FIELD: recordFreq
178  00 00 00 0a                 10                length of name
182  72 65 63 6f 72 64 46 72     "recordFr"        field name
190  65 71                       "eq"   
192  00 03                       3 (DOUBLE)        data type   
194  00 00                       0                 number of dimensions   
196  00 00 00 02                 2                 length of units   
200  48 7a                       "Hz"              units   
202  3f f0 00 00 00 00 00 00     1.0               scale
210  00 00 00 00 00 00 00 00     0.0               offset
218  00 00 00 00                 0                 number of axis names
222  00 01                       1 (GLOBAL)        occurrence class
224  40 bf 40 00 00 00 00 00     8000.0            data
232  00 00 00 00                 0                 number of subfields
                        FOURTH FIELD: samples
236  00 00 00 07                 7                 length of name
240  73 61 6d 70 6c 65 73        "samples"         field name
247  00 07                       7 (SHORT)         data type
249  00 01                       1                 number of dimensions
251  00 00 00 01                 1                 dimension
255  00 00 00 00                 0                 length of units

so we might 'read' these guys by getting the header length from the preamble,
and the data type by searching blindly for "samples" followed by the type
indication, then just guess at a sampling rate.
*/
#endif

#if 0
/* MIME uses base64 encoding which is something like: */
int decode_char (unsigned char c)
{
  if (isupper (c)) return(c - 'A'); 
  else if (islower (c)) return(c - 'a' + 26); 
  else if (isdigit (c)) return(c - '0' + 52);
  else if (c == '+') return(62); 
  else if (c == '/') return(63); 
  return(0);
}

from_base64(char* cs, int *buf)
{
  /* assume we grab 4 chars and set 3 ints on each call */
  int dc1,dc2,dc3,dc4;
  dc1=decode_char(cs[0]);  dc2=decode_char(cs[1]);  dc3=decode_char(cs[2]);  dc4=decode_char(cs[3]);
  buf[0]=((dc1<<2) | (dc2>>4));  buf[1]=(((dc2&0xf)<<4) | (dc3>>2));  buf[2]=(((dc3&0x3)<<6) | dc4);
}
#endif

/* ILS headers (info from P Kabal): Interactive Laboratory System.
 * in 1988 info from ils@hub.ucsb.edu
 * v4 is in longs, v3 in shorts
 * data is 16-bit linear (probably big-endian) (locs below are in version format)
 * 0: #pts in analysis window, 1: #autoregressive coeffs, 2: preemphasis, 3: shift per frame, 4: hamming if 'y'
 * 5: #data blocks, 6: #resonances, 7: start frame, 8: #frames, 9: start sector, 10: #autoregressive coeffs
 * 11: #pts past whole block, 12-15: 'field chars', 16: #frames, 17: autoreg flag, 18: #disk num of file
 * 19: another disk number, 20: analysis mnemonic, 22-26: id chars, 35-36: scaling mult, 37-38: scaling adder
 * 57: starting chan, 58: #chans, 59: mulaw if 50, 60: power of ten mult for sampling freq, 61: int srate,
 * 62: -29000 if not sampled, -32000 if sampled, 63: 32149 if initialized.
 * 128: data starts (or 256 in v3, I think) -- byte 512 in any case.
 */

/* for MIDI and Synth-related formats by the dozen see http://www.wotsit.demon.co.uk/music.htm */

/*
 * real audio: http://www1.real.com/devzone/sdks/rmsdk/guide/index.html
*/
/*
{
int i;
lseek(chan,0,SEEK_SET);
read(chan,hdrbuf,HDRBUFSIZ); 
for (i=0;i<HDRBUFSIZ;i++)
  {
    fprintf(stderr,"%d:   %d %c    %d %d     %d %d\n",
	    i,
	    hdrbuf[i],hdrbuf[i],
	    get_little_endian_short((unsigned char *)(hdrbuf+i)),
	    get_big_endian_short((unsigned char *)(hdrbuf+i)),
	    get_little_endian_int((unsigned char *)(hdrbuf+i)),
	    get_big_endian_int(unsigned char *)(hdrbuf+i));
  }
}
*/
