#ifdef USE_SNDLIB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "../H/sndlibsupport.h"

/* Peak amps encoded at end of comment as follows:
      "[peakamp: <peak> at <loc>, <peak> at <loc>]"
   where <peak> is a floating point peak amp and <loc> is an integer
   frame number; one pair for each channel.
   For example:
      "[peakamp: 32767.0 at 1037829, 27498.58 at 1048292]"
*/
#define PEAK_TAG      "[peakamp:"

static int update_current_header_comment(char *, char *, int);



/* -------------------------------------------- sndlib_get_header_comment --- */
int
sndlib_get_header_comment(char *sfname, SFComment *sfc)
{
   create_header_buffer();                 /* only inits once */
   c_read_header(sfname);
   return (sndlib_get_current_header_comment(sfname, sfc));
}


/* ------------------------------------ sndlib_get_current_header_comment --- */
/* Fills in an SFComment struct for the given sound file.
   Returns -1 if there's an error; otherwise returns 0.

   Reads the comment from the file, and checks to see if it encodes
   peak stats in the format given at the top of this module. If it
   does, then parses the stats and stores them into the <peak> and
   <peakloc> fields of the SFComment struct. Stores the comment text
   preceding -- but not including -- the peak stat text into the
   <comment> field of the SFComment struct.

   NOTE: Text following the peak stats will be ignored.

   If there is no comment in the file, zeros the <comment> field and
   returns 0. If there are no parseable peak stats (and there can't
   be if there's no comment!), sets the <offset> field to -1 and
   returns 0.

   If the comment appears to have peak stats, but we fail to parse them,
   the <comment> field will contain as much of the file's comment text as
   it will hold; we print a warning and set <offset> to -1.

   Note that there can be peak stats AND an empty <comment> field, in the
   (very common) case where the file's comment contains only peak stats.

   Assumes we've already called create_header_buffer and c_read_header for
   this file.
*/
int
sndlib_get_current_header_comment(char *sfname, SFComment *sfc)
{
   int       n, start, end, len, fd, maxchars, bytes, found;
   char      *pstr;
   char      buf[MAX_COMMENT_CHARS + MAX_PEAK_CHARS];

   sfc->offset = -1;
   sfc->comment[0] = '\0';

   start = c_snd_header_comment_start();
   end = c_snd_header_comment_end();
   len = end - start + 1;

   if (len <= 1)
      return 0;                /* no comment text at all (not an error) */

   maxchars = MAX_COMMENT_CHARS + MAX_PEAK_CHARS;
   if (len >= maxchars)
      len = maxchars - 1;

   fd = open(sfname, O_RDONLY);
   if (fd == -1) {             /* not likely, since we've opened it before */
      fprintf(stderr, "sndlib_get_current_header_comment: can't open %s (%s)\n",
                                                     sfname, strerror(errno));
      return -1;
   }
   lseek(fd, start, SEEK_SET);
   bytes = read(fd, buf, len);
   buf[len] = '\0';                              /* ensure termination */
   close(fd);

   /* we should've read the amount sndlib said is there */
   if (bytes < len) {
      fprintf(stderr, "sndlib_get_current_header_comment: read failed!\n");
      return -1;
   }

   if (buf[0] == '\0')                    /* empty comment (not an error) */
      return 0;

   /* See if comment contains peak info. */

   found = 0;
   pstr = buf;
   while (!found) {
      pstr = strrchr(pstr, '[');     /* search backwards from end of string */
      if (pstr == NULL)
         break;
      if (strncmp(pstr, PEAK_TAG, strlen(PEAK_TAG)) == 0) {
         found = 1;
         break;
      }
      if (pstr == buf)        /* '[' at beginning of string, but no PEAK_TAG */
         break;
   }

   /* Copy buf into sfc->comment. This will contain as much of the real
      file comment as will fit, including any encoded peak stats. If we
      successfully parse the peak stats below, we'll chop them off
      the end of sfc->comment. (We don't alter the file, of course!)
   */
   strncpy(sfc->comment, buf, MAX_COMMENT_CHARS - 1);
   sfc->comment[MAX_COMMENT_CHARS - 1] = '\0';         /* ensure termination */

   /* If we found the peak info tag, parse the statement it introduces, and
      store the peak values into our SFComment struct.  The error checking
      below seems paranoid, but that's because a user could mangle the peak
      info while editing a comment.
   */
   if (found) {
      int   nchans;
      char  *p;

      sfc->offset = pstr - buf;

      p = strchr(pstr, ':');
      if (p == NULL)
         goto parse_err;
      p++;

      nchans = c_snd_header_chans();

      for (n = 0; n < nchans; n++) {
         double  peak;
         long    peakloc;
         char    *pos;

         /* read peak amp value */
         pos = NULL;
         errno = 0;
         peak = strtod(p, &pos);
         if (peak == 0.0 && pos == p)      /* no conversion performed */
            goto parse_err;
         if (errno == ERANGE)              /* overflow or underflow */
            goto parse_err;
         sfc->peak[n] = (float)peak;

         /* find peak location value */
         p = strchr(p, 'a');               /* find "at" */
         if (p == NULL)
            goto parse_err;
         if (*(p+1) != 't')
            goto parse_err;
         p += 2;                           /* skip over "at" */

         /* read peak location value */
         pos = NULL;
         errno = 0;
         peakloc = strtol(p, &pos, 10);
         if (pos == p)                     /* no digits to convert */
            goto parse_err;
         if (errno == ERANGE)              /* overflow or underflow */
            goto parse_err;
         sfc->peakloc[n] = peakloc;

         /* skip to info for next channel, or stop */

         if (*pos == ']')                  /* reached end of peak info */
            break;                         /* even if file has more chans */

         p = pos + 1;                      /* skip over ',' separating chans */
      }

      /* truncate comment to omit peak info (and anything after!) */
      sfc->comment[sfc->offset] = '\0';
   }

   return 0;

parse_err:
   sfc->offset = -1;
   fprintf(stderr, "WARNING: Can't parse peak amp stats for \"%s\"\n", sfname);
   return 0;     /* still not really an error! */
}


/* -------------------------------------------- sndlib_set_header_comment --- */
/* Update the header of the existing sound file <sfname> using the supplied
   values for peak amplitudes and their locations (in frames from beginning of
   file), and the <comment> string. The <peak> and <peakloc> arrays have
   an element for each channel in the sound file. If <peak> or <peakloc>
   is NULL, don't update the file's peak amp stats. If <comment> is NULL,
   don't update the comment.
   Note that the peak amp stats are encoded as part of the file comment.
   (See above for an explanation of this scheme.)
   Returns 0 on success, -1 on failure.
*/
int
sndlib_set_header_comment(char   *sfname,
                          float  peak[],
                          long   peakloc[],
                          char   *comment)
{
   int       fd, n, nchans, result;
   char      commstr[MAX_COMMENT_CHARS + MAX_PEAK_CHARS];
   char      tmp[256], peakstr[MAX_PEAK_CHARS];
   SFComment sfc;

   /* see if file exists and we can write it */
   fd = open(sfname, O_RDWR);
   if (fd == -1) {
      fprintf(stderr, "sndlib_set_header_comment: \"%s\" (%s)\n",
                                                     sfname, strerror(errno));
      return -1;
   }
   close(fd);
      
   result = sndlib_get_header_comment(sfname, &sfc);
   if (result == -1)
      return -1;

   nchans = c_snd_header_chans();

   /* prepare peak stats string */
   peakstr[0] = 0;
   if (peak && peakloc) {                         /* use supplied peak stats */
      strcat(peakstr, PEAK_TAG);
      for (n = 0; n < nchans; n++) {
         sprintf(tmp, " %.9g at %d,", peak[n], peakloc[n]);
         strcat(peakstr, tmp);
      }
      peakstr[strlen(peakstr) - 1] = ']';         /* replace last ',' */
   }
   else if (sfc.offset > -1) {                    /* use peak stats in file */
      strcat(peakstr, PEAK_TAG);
      for (n = 0; n < nchans; n++) {
         sprintf(tmp, "%.9g at %d,", sfc.peak[n], sfc.peakloc[n]);
         strcat(peakstr, tmp);
      }
      peakstr[strlen(peakstr) - 1] = ']';         /* replace last ',' */
   }
   /* else peak stats empty */

   /* prepare comment string */
   if (comment) {
      strncpy(commstr, comment, MAX_COMMENT_CHARS - 1);
      commstr[MAX_COMMENT_CHARS - 1] = '\0';      /* ensure termination */
   }
   else if (sfc.comment[0]) {
      strncpy(commstr, sfc.comment, MAX_COMMENT_CHARS - 1);
      commstr[MAX_COMMENT_CHARS - 1] = '\0';      /* ensure termination */
   }
   else
      commstr[0] = '\0';                          /* no comment */

   /* append peak stats string to comment string */
   if (peakstr[0] && commstr[0])
      strcat(commstr, "\n");        /* separate real comment from peak stats */
   strcat(commstr, peakstr);

   /* Write entire comment (incl. peak stats) to file header.
      Note that we tell sndlib to keep the full comment allocation,
      even if the actual comment is shorter.
   */
   result = update_current_header_comment(sfname, commstr,
                                                  DEFAULT_COMMENT_LENGTH - 1);
   if (result) {
      fprintf(stderr, "sndlib_set_header_comment: can't save comment!\n");
      return -1;
   }

   return 0;
}


/* ---------------------------------------------------- sndlib_close_file --- */
/* This is like clm_close (sndlib/io.c), except that it allows us to check
   the return value of close. (man 3 close for the reason.)
*/
int
sndlib_close_file(int fd)
{
   close_clm_file_descriptors(fd);
   return close(fd);
}


/* ---------------------------------------- update_current_header_comment --- */
/* We need this because sndlib doesn't provide a way to update the header
   comment after the file has been created.  Note that c_update_header_comment
   writes an annotation string to the *end* of the file (for formats that
   support that) -- not what we want.

   <comment_len> is the length of <comment>, not including terminating NULL
   (i.e., what you'd get from strlen).

   Assumes we've already called create_header_buffer and c_read_header for
   the current file.  Adapted from c_write_header (sndlib/header.c).
*/
static int
update_current_header_comment(char *sfname, char *comment, int comment_len)
{
   int  fd, result, header_type, srate, chans;
   int  data_location, data_size, data_format;
   int  comment_start, comment_end;

   header_type = c_snd_header_type();
   data_format = c_snd_header_format();
   srate = c_snd_header_srate();
   chans = c_snd_header_chans();
   data_location = c_snd_header_data_location();
   data_size = c_snd_header_data_size();
   comment_start = c_snd_header_comment_start();
   comment_end = c_snd_header_comment_end();

   if (comment_end - comment_start < comment_len) {
      fprintf(stderr, "update_current_header_comment: ");
      fprintf(stderr, "not enough space for comment in \"%s\"\n", sfname);
      return -1;
   }

   fd = open(sfname, O_RDWR);
   if (fd == -1) {
      fprintf(stderr, "update_current_header_comment: can't open \"%s\"\n",
                                                                      sfname);
      return -1;
   }

   result = c_write_header_with_fd(fd, header_type, srate, chans,
                                   data_location, data_size, data_format,
                                   comment, comment_len);
   if (result == -1) {
      fprintf(stderr, "update_current_header_comment: ");
      fprintf(stderr, "can't update header for \"%s\"\n", sfname);
      return -1;
   }

   result = close(fd);
   if (result == -1) {
      fprintf(stderr, "update_current_header_comment: close failed: %s (%s)\n",
                                                     sfname, strerror(errno));
      return -1;
   }

   return 0;
}


#endif /* USE_SNDLIB */

