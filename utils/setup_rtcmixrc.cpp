/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
/* Write a basic .rtcmixrc file in the user's home directory.  -JGG, 7/1/04 */

#include <Option.h>
#include <stdio.h>

int main()
{
   Option::init();
   if (Option::writeConfigFile(Option::rcName()) != 0)
      return -1;
   printf("Configuration file \"%s\" successfully written.\n",
                                                         Option::rcName());
   return 0;
}


/* We make our own warn function so that we don't have to pull in more
   RTcmix code.  This must have the same signature as the real one in
   message.c.  It doesn't print WARNING ***, though.
*/
#include <stdarg.h>

#define BUFSIZE 1024

extern "C" {
void
warn(const char *inst_name, const char *format, ...)
{
   // ignore inst_name

   char     buf[BUFSIZE];
   va_list  args;

   va_start(args, format);
   vsnprintf(buf, BUFSIZE, format, args);
   va_end(args);

   fprintf(stderr, "\n%s\n\n", buf);
}
} // extern "C"

