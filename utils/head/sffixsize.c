#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sndlibsupport.h>

static void usage(void);



/* ---------------------------------------------------------------- usage --- */
static void
usage()
{
#ifdef NOTYET
   printf("usage:  \"sffixsize [options] file [file...]\"\n");
   printf("        options:  -v verbose\n");
#else
   printf("usage:  \"sffixsize file [file...]\"\n");
#endif
   printf("        This program fixes the data size field in sound file\n");
   printf("        headers that weren't updated correctly.\n");
   exit(1);
}


/* ----------------------------------------------------------------- main --- */
int
main(int argc, char *argv[])
{
   int         i, fd, header_type, data_location, verbose = 0;
   int         nsamps, result, nchans, sound_bytes;
   off_t       true_file_length;
   char        *sfname;
   struct stat statbuf;

   if (argc < 2)
      usage();

   for (i = 1; i < argc; i++) {
      char *arg = argv[i];

      if (arg[0] == '-') {
         switch (arg[1]) {
            case 'v':
               verbose = 1;
               break;
            default:
               usage();
         }
         continue;
      }
      sfname = arg;

      /* see if file exists and we can read it */

      fd = open(sfname, O_RDWR);
      if (fd == -1) {
         fprintf(stderr, "%s: %s\n", sfname, strerror(errno));
         continue;
      }

      /* make sure it's a regular file or symbolic link */

      result = fstat(fd, &statbuf);
      if (result == -1) {
         fprintf(stderr, "%s: %s\n", sfname, strerror(errno));
         continue;
      }
      if (!S_ISREG(statbuf.st_mode) && !S_ISLNK(statbuf.st_mode)) {
         fprintf(stderr, "%s is not a regular file or a link.\n", sfname);
         continue;
      }
 
      /* read header and gather the data */

      result = sndlib_read_header(fd);
      if (result == -1) {
         fprintf(stderr, "\nCan't read header of \"%s\"!\n\n", sfname);
         close(fd);
         continue;
      }
      header_type = mus_header_type();
      if (NOT_A_SOUND_FILE(header_type)) {
         fprintf(stderr, "\"%s\" is probably not a sound file\n", sfname);
         close(fd);
         continue;
      }
      if (!WRITEABLE_HEADER_TYPE(header_type)) {
         fprintf(stderr, "\"%s\": can't update this type of header.\n", sfname);
         close(fd);
         continue;
      }
      data_location = mus_header_data_location();
#ifdef NOT_NEEDED
      nchans = mus_header_chans();
      nsamps = mus_header_samples();           /* samples, not frames */
#endif

      true_file_length = lseek(fd, 0, SEEK_END);
      if (true_file_length == -1) {
         perror("lseek");
         exit(1);
      }
// FIXME: This is wrong for files that have header chunks following the
// sound data chunk (SSND), such as those written by the Peak program.
      sound_bytes = true_file_length - data_location;

      result = sndlib_set_header_data_size(fd, header_type, sound_bytes);
      if (result == -1) {
         fprintf(stderr, "Error updating header\n");
         exit(1);
      }

      close(fd);

   }

   return 0;
}

