# Process this file with autoconf to produce a configure script.

# Our strategy is to populate a file, defs.conf, with just the bare
# minimum of makefile variables that will obviate the need for users
# to edit makefile.conf and site.conf manually.  Users with special
# needs can override or augment the settings in defs.conf by editing
# site.conf.  We could be taking all sorts of other standard variables
# from the configure process, but we don't, at least for now.
# -JGG, 7/29/04

AC_INIT(RTcmix, 4.0)
AC_CONFIG_SRCDIR(src/rtcmix/main.cpp)

CMIXDIR=`pwd`
AC_SUBST(CMIXDIR)

# Determine host ---------------------------------------------------------------
# NB: This requires that we distribute install-sh, config.sub and config.guess.
AC_CANONICAL_HOST
case "$host" in
   *-*-linux*)
      ARCH=LINUX
      ;;
   *-apple-*)
      ARCH=MACOSX
      ;;
   *-sgi*)
      ARCH=SGI
      ;;
   *-*-freebsd*)
      ARCH=FREEBSD
      ;;
   *)
      ARCH=LINUX
      ;;
esac
AC_SUBST(ARCH)


# Configuration options --------------------------------------------------------
#
# Standard format for options is...
#    --with-option         (equivalent to --with-option=yes)
#    --without-option      (equivalent to --with-option=no)
#    --with-option=ARG
# The built-in var <withval> will be set to "yes", "no" or "ARG" accordingly.
# For perl and python options, we must be prepared to see any one of these
# three values of <withval>.  -JGG
#
# Note that the order of options appearing in "configure --help" output 
# follows the order of AC_ARG_WITH() calls below.

# Audio driver options
AUDIODRIVER=AUTO                          # default
AC_ARG_WITH(alsa, AC_HELP_STRING([--with-alsa],
   [use ALSA audio driver]),
   [if test "$withval" != "no"; then
      AUDIODRIVER=ALSA              # this is ignored on irrelevant platforms
   fi])
AC_SUBST(AUDIODRIVER)

# Perl options
PERL_SUPPORT=FALSE                        # disable by default
PERL=perl                                 # default perl name
AC_ARG_WITH(perl, AC_HELP_STRING([--with-perl=PATH],
   [build Perl-enabled RTcmix (PCMIX) (PATH optional)]),
   [if test "$withval" != "no"; then
      if test "$withval" != "yes"; then
         PERL="$withval"                  # the path to perl
      fi
      PERL_SUPPORT=TRUE
   fi])
AC_SUBST(PERL_SUPPORT)
AC_SUBST(PERL)

# Python options
PYTHON_SUPPORT=FALSE                      # disable by default
PYTHON=python                             # default python name
AC_ARG_WITH(python, AC_HELP_STRING([--with-python=PATH],
   [build Python-enabled RTcmix (PYCMIX) (PATH optional)]),
   [if test "$withval" != "no"; then
      if test "$withval" != "yes"; then
         PYTHON="$withval"                # the path to python
      fi
      PYTHON_SUPPORT=TRUE
   fi])
AC_SUBST(PYTHON_SUPPORT)
AC_SUBST(PYTHON)

# Documentation options
HTMLDESTDIR="$CMIXDIR"/docs/html          # default location
AC_ARG_WITH(docdir, AC_HELP_STRING([--with-docdir=PATH],
   [put documentation in this directory]),
   [if test "$withval" != "no" && test "$withval" != "yes"; then
      AC_SUBST(HTMLDESTDIR, ["$withval"/html]),
   fi])
AC_SUBST(HTMLDESTDIR)

# NetPlay options
NPLAY_SUPPORT=FALSE                       # default
AC_ARG_WITH(netplay, AC_HELP_STRING([--with-netplay],
   [support NetPlay]),
   [if test "$withval" != "no"; then
      NPLAY_SUPPORT=TRUE
   fi])
AC_SUBST(NPLAY_SUPPORT)

#AC_SUBST(WITH_VAL, "$withval")    # for debugging this script


# Standard program, library and header checks ----------------------------------

# Check for programs.
AC_PROG_CC
AC_PROG_CXX

# Check for libraries.
AC_PATH_X
AC_SUBST(x_libraries)

# Check for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h float.h limits.h mach/mach.h netdb.h netinet/in.h \
stddef.h stdint.h stdlib.h string.h strings.h sys/file.h sys/ioctl.h \
sys/param.h sys/socket.h sys/time.h termios.h unistd.h values.h)

# Check for typedefs, structures, and compiler characteristics.
# Check for library functions.


# Our own program, library and header checks -----------------------------------

# Test for presence of alsa lib, header, version
if test "$AUDIODRIVER" = "ALSA"; then
   AC_CHECK_HEADER(alsa/asoundlib.h, [], AUDIODRIVER=OSS)
   if test "$AUDIODRIVER" = "OSS"; then
      AC_MSG_WARN([ALSA asoundlib.h not present; using OSS instead of ALSA])
   fi
   AC_CHECK_LIB(asound, snd_pcm_open, [], AUDIODRIVER=OSS)
   if test "$AUDIODRIVER" = "OSS"; then
      AC_MSG_WARN([ALSA library not present; using OSS instead of ALSA])
   fi
   if test "$AUDIODRIVER" = "ALSA"; then
      # test for ALSA version >= 0.9; if not, warn and use OSS (test from snd-6)
      alsa_ok=yes
      AC_MSG_CHECKING([for ALSA 0.9 or later])
      AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM([#include <alsa/asoundlib.h>],
         [
            #if ((SND_LIB_MAJOR == 0) && (SND_LIB_MINOR < 9))
               #error too old
            #endif
         ])],
         alsa_ok=yes,
         alsa_ok=no)
      AC_MSG_RESULT($alsa_ok)
      if test "$alsa_ok" != "yes"; then
         AC_MSG_WARN([ALSA version must be >= 0.9; using OSS instead of ALSA])
         AUDIODRIVER=OSS
      fi
   fi
fi


# Write output files -----------------------------------------------------------
#AC_CONFIG_HEADER(config.h)   # we don't need this yet
AC_CONFIG_FILES(makefile.conf)
AC_CONFIG_FILES(defs.conf)
AC_OUTPUT

