#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sndlibsupport.h>
#include <sfheader.h>
#include <byte_routines.h>

extern int swap;    /* defined in sys/check_byte_order.c */

int
main(int argc, char *argv[])
{
   int         fd, result;
   struct stat sfst;
   size_t      nbytes;
   double      amount;
   char        *sfname;
   SFHEADER    sfh;

   if (argc != 3) {
      printf("usage: sfshrink new_duration_in_seconds filename\n");
      printf("       (to extend with zeros, add to current duration)\n");
      exit(1);
   }
   amount = atof(*++argv);
   sfname = *++argv;
   rwopensf(sfname, fd, sfh, sfst, "sfshrink", result, O_RDWR)
   if (result == -1)
      exit(1);
   nbytes = amount * sfsrate(&sfh) * sfclass(&sfh) * sfchans(&sfh)
                                                     + getheadersize(&sfh);
   nbytes -= nbytes % (sfclass(&sfh) * sfchans(&sfh));
   if (ftruncate(fd, nbytes) == -1)
      perror("sfshrink: ftruncate");
   else
      putlength(sfname, fd, &sfh);
   close(fd);

   return 0;
}

