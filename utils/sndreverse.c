/* This is an old cmix version of an IRCAM program from 1985. Updating it for
   current RTcmix with sndlib support (but still using legacy cmix API).  -JGG
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sfheader.h>
#include <byte_routines.h>
#include <sndlibsupport.h>

/* This program is designed to reverse existing soundfiles.
   The channels of the soundfile are kept in the original order
   contrary to what an analog tape would do. */

extern int swap;

int
main(int argc, char *argv[])
{
   int      i, sfd1, sfd2, backsu, units, nsu, nextsu, result, readbyte;
   off_t    bytes;
   char     *forward, *back;
   char     outfilename[1024], infilename[1024];
   SFHEADER hd1, hd2;
   struct stat st;

   if (argc != 2) {
      fprintf(stderr, "Usage:  %s <filename>\n", argv[0]);
      exit(1);
   }

   strcpy(infilename, argv[1]);          /* Don't install setuid root  ;-) */

   readopensf(infilename, sfd1, hd1, st, "sndreverse", result);
   if (result < 0)
      exit(1);

   strcpy(outfilename, infilename);
   strcat(outfilename, ".r");

   if ((sfd2 = open(outfilename, O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0) {
      fprintf(stderr, "Error creating file \"%s\".\n", outfilename);
      close(sfd1);
      exit(1);
   }

   /* Duplicate input file header and write it to new output file. */
   hd2 = hd1;
   sfdatasize(&hd2) = 0;
   if (wheader(sfd2, &hd2)) {
      fprintf(stderr, "Write to header failed.\n");
      close(sfd1);
      close(sfd2);
      exit(1);
   }

   /* Close and reopen output file to get updated SFHEADER. */
   close(sfd2);
   rwopensf(outfilename, sfd2, hd2, st, "sndreverse", result, O_RDWR);
   if (result < 0)
      exit(1);

   /* Get memory for buffers */
   forward = (char *) malloc(SF_BUFSIZE);
   back = (char *) malloc(SF_BUFSIZE);
   if (!forward || !back) {
      fprintf(stderr, "Bad allocation for buffers.\n");
      exit(1);
   }

   bytes = sfdatasize(&hd1);
   readbyte = bytes % SF_BUFSIZE;
   if (lseek(sfd1, (long) -readbyte, SEEK_END) == -1) {
      fprintf(stderr, "Bad seek.\n");
      exit(1);
   }

   while (bytes > 0) {          /* For all of the samples */
      if ((readbyte = read(sfd1, forward, SF_BUFSIZE)) < 0) {
         fprintf(stderr, "Bad read on soundfile \"%s\".\n", infilename);
         exit(1);
      }
      /* units = samples per read.
         nsu = number of sample units per read. 
         (sample unit = all channels
         for this sample slot).
         nextsu = next sample unit.
         backsu = current backward sample unit 
         starting channel. */

      units = readbyte / sfclass(&hd1);
      nextsu = 0;
      nsu = units - sfchans(&hd1);
      if (sfclass(&hd1) == SF_FLOAT) {  /* Do floating point file */
         register float *fbuf = (float *) back;
         float *fforward = (float *) forward;

         while ((char *) fbuf < back + readbyte) {
            backsu = nsu - nextsu;
            for (i = 0; i < sfchans(&hd1); i++) {
               *fbuf++ = *(fforward + backsu + i);
               if (swap) {
                  byte_reverse4(fbuf);
               }
            }
            nextsu += i;
         }
      }
      else {                    /* Integer (short) file */
         register short *sbuf = (short *) back;
         short *sforward = (short *) forward;

         while ((char *) sbuf < back + readbyte) {
            backsu = nsu - nextsu;
            for (i = 0; i < sfchans(&hd1); i++) {
               *sbuf++ = *(sforward + backsu + i);
               if (swap) {
                  byte_reverse2(sbuf);
               }
            }
            nextsu += i;
         }
      }

      if (write(sfd2, back, readbyte) != readbyte) {
         fprintf(stderr, "Bad write on soundfile.\n");
         exit(1);
      }
      if ((bytes -= readbyte) > 0)
         if (lseek(sfd1, (long) -(SF_BUFSIZE + readbyte), SEEK_CUR) == -1) {
            fprintf(stderr, "Bad seek.\n");
            exit(1);
         }
   }

   /* Update header for bytes of sound data written. */
   putlength(outfilename, sfd2, &hd2);

   close(sfd1);
   close(sfd2);

   return 0;
}

