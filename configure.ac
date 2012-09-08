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
#
# There is a similar set of flags for enabling and disabling features...
#    --enable-feature
#    --disable-feature

# example of using --enable-foo construction
#PTRSIZEFLAG=
#AC_ARG_ENABLE(64bit,
#   AS_HELP_STRING([--enable-64bit], [build 64-bit version]),
#   [if test "$enableval" == "yes"; then
#      TMP_CFLAGS="$CFLAGS"
#      CFLAGS="-m64"
#      AC_RUN_IFELSE(
#         [AC_LANG_PROGRAM([], [return sizeof(void *) == 8 ? 0 : 1;])],
#         [PTRSIZEFLAG="-m64"],
#         [AC_MSG_ERROR([Don't know how to build a 64-bit object.])]
#      )
#      CFLAGS="$TMP_CFLAGS"
#   elif test "$enableval" == "no"; then
#      PTRSIZEFLAG="-m32"
#   fi]
#)
#AC_SUBST(PTRSIZEFLAG)

#############################################################################
# Architecture -- pointer size
# This is set up to allow compiling fat (64/32) binaries eventually.  -JG

# First find out which ptr sizes we can support
# NB: Must do AC_PROG_CC above this, or it will fail on Slack 10.2.
TMP_CFLAGS="$CFLAGS"
CFLAGS="-m64"
AC_RUN_IFELSE(
   [AC_LANG_PROGRAM([], [return sizeof(void *) == 8 ? 0 : 1;])],
   [CAN_BUILD_64BIT="yes"],
   [CAN_BUILD_64BIT="no"]
)
CFLAGS="-m32"
AC_RUN_IFELSE(
   [AC_LANG_PROGRAM([], [return sizeof(void *) == 4 ? 0 : 1;])],
   [CAN_BUILD_32BIT="yes"],
   [CAN_BUILD_32BIT="no"]
)
CFLAGS="$TMP_CFLAGS"

# Gather user requests
WANTS_64BIT=""
AC_ARG_WITH(64bit,
   AC_HELP_STRING([--with-64bit], [build 64-bit version]),
   [if test "$withval" != "no"; then
      if test "$CAN_BUILD_64BIT" = "no"; then
         AC_MSG_ERROR([Don't know how to build a 64-bit object.])
      fi
      WANTS_64BIT="yes"
   else
      WANTS_64BIT="no"
   fi]
)
WANTS_32BIT=""
AC_ARG_WITH(32bit,
   AC_HELP_STRING([--with-32bit], [build 32-bit version]),
   [if test "$withval" != "no"; then
      if test "$CAN_BUILD_32BIT" = "no"; then
         AC_MSG_ERROR([Don't know how to build a 32-bit object.])
      fi
      WANTS_32BIT="yes"
   else
      WANTS_32BIT="no"
   fi]
)

# Build only 64bit by default, unless user requests 32bit, or requests not to
# build 64bit, or the system doesn't support 64bit. This logic will have to be
# changed if we want to support fat 64/32 binaries.
ARCH_64BIT=""
ARCH_32BIT=""
if test "$CAN_BUILD_64BIT" = "yes" \
		-a "$WANTS_32BIT" != "yes" \
		-a "$WANTS_64BIT" != "no"; then
  	ARCH_64BIT="-m64"
else
   if test "$CAN_BUILD_32BIT" = "no"; then
      AC_MSG_ERROR([Don't know how to build a 32-bit object.])
   fi
   ARCH_32BIT="-m32"
fi

# Set up summary display for user and export variables.
WITH_64BIT="false"
WITH_32BIT="false"
if test "$ARCH_64BIT" != ""; then
   WITH_64BIT="true"
fi
if test "$ARCH_32BIT" != ""; then
   WITH_32BIT="true"
fi
#echo "****** ARCH_64BIT: $ARCH_64BIT" 
#echo "****** ARCH_32BIT: $ARCH_32BIT" 

# Combine the bit flags into one symbol to be added to ARCHFLAGS later
ARCH_BITFLAGS="$ARCH_64BIT $ARCH_32BIT"
AC_SUBST(ARCH_BITFLAGS)

# Feed the right arch flags to Xcode for MouseWindow, DisplayWindow
BUILD_ARCH=""
if test "$ARCH" = "MACOSX"; then  # this trips up slack 10.2, so don't bother
   if test "$WITH_32BIT" = "true"; then
      BUILD_ARCH+="i386"
   fi
   if test "$WITH_64BIT" = "true"; then
      BUILD_ARCH+="x86_64"
   fi
fi
AC_SUBST(BUILD_ARCH)
#############################################################################

# Enable running instruments on different processors
MULTI_THREAD_SUPPORT=FALSE                # disable by default
THREAD_COUNT=2                            # default number of threads
WITH_MULTI_THREAD="false"                 # just for summary display below
AC_ARG_WITH(multithread,
   AC_HELP_STRING([--with-multithread=THREADCOUNT],
         [build multi-threaded RTcmix on Linux or OS X 10.6 or later (THREADCOUNT optional, default=2)]),
	[if test "$ARCH" != "MACOSX" && test "$ARCH" != "LINUX"; then
      AC_MSG_ERROR([Can't build multi-threaded version unless on Linux, or OS X 10.6 or later.])
	fi]
   [if test "$withval" != "no"; then
      if test "$withval" != "yes"; then
         THREAD_COUNT="$withval"
      fi
      MULTI_THREAD_SUPPORT=TRUE
      WITH_MULTI_THREAD="true"
   fi]
)
AC_SUBST(MULTI_THREAD_SUPPORT)
AC_SUBST(THREAD_COUNT)

# Audio driver options
AUDIODRIVER=AUTO                          # default
WITH_ALSA="false"                         # just for summary display below
AC_ARG_WITH(alsa,
   AC_HELP_STRING([--with-alsa], [use ALSA audio driver]),
   [if test "$withval" != "no"; then
      AUDIODRIVER=ALSA              # this is ignored on irrelevant platforms
      WITH_ALSA="true"
   fi]
)
AC_SUBST(AUDIODRIVER)

# Perl options
PERL_SUPPORT=FALSE                        # disable by default
PERL_IN_LOCAL=FALSE
PERL=perl                                 # default perl name
WITH_PERL="false"                         # just for summary display below
AC_ARG_WITH(perl,
   AC_HELP_STRING([--with-perl=PATH],
         [build Perl-enabled RTcmix (PCMIX) (PATH optional)]),
   [if test "$withval" != "no"; then
      if test "$withval" != "yes"; then
         PERL="$withval"                  # the path to perl
      fi
      PERL_SUPPORT=TRUE
      WITH_PERL="true"
      if test PERL = "/usr/local/bin/perl"; then
         PERL_IN_LOCAL=TRUE
      fi
   fi]
)
AC_SUBST(PERL_SUPPORT)
AC_SUBST(PERL_IN_LOCAL)
AC_SUBST(PERL)

# Python options
PYTHON_SUPPORT=FALSE                      # disable by default
PYTHON=python                             # default python name
WITH_PYTHON="false"                       # just for summary display below
AC_ARG_WITH(python,
   AC_HELP_STRING([--with-python=PATH],
         [build Python-enabled RTcmix (PYCMIX) (PATH optional)]),
   [if test "$withval" != "no"; then
      if test "$withval" != "yes"; then
         PYTHON="$withval"                # the path to python
      fi
      PYTHON_SUPPORT=TRUE
      WITH_PYTHON="true"
   fi]
)
AC_SUBST(PYTHON_SUPPORT)
AC_SUBST(PYTHON)

# Documentation options
HTMLDESTDIR="$CMIXDIR"/docs/html          # default location
AC_ARG_WITH(docdir,
   AC_HELP_STRING([--with-docdir=PATH], [put documentation in this directory]),
   [if test "$withval" != "no" && test "$withval" != "yes"; then
      AC_SUBST(HTMLDESTDIR, ["$withval"/html]),
   fi]
)
AC_SUBST(HTMLDESTDIR)

# NetPlay options
NPLAY_SUPPORT=FALSE                       # default
WITH_NPLAY="false"                        # just for summary display below
AC_ARG_WITH(netplay,
   AC_HELP_STRING([--with-netplay], [support NetPlay]),
   [if test "$withval" != "no"; then
      NPLAY_SUPPORT=TRUE
      WITH_NPLAY="true"
   fi]
)
AC_SUBST(NPLAY_SUPPORT)

# JACK options
JACK_SUPPORT=FALSE                        # default
WITH_JACK="false"                         # just for summary display below
AC_ARG_WITH(jack,
   AC_HELP_STRING([--with-jack], [support JACK]),
   [if test "$withval" != "no"; then
      JACK_SUPPORT=TRUE
      WITH_JACK="true"
   fi]
)
AC_SUBST(JACK_SUPPORT)

# FFTW options
FFTW_SUPPORT=FALSE                        # default
WITH_FFTW="false"                         # just for summary display below
AC_ARG_WITH(fftw,
   AC_HELP_STRING([--with-fftw], [use FFTW (alternative FFT library)]),
   [if test "$withval" != "no"; then
      FFTW_SUPPORT=TRUE
      WITH_FFTW="true"
   fi]
)
AC_SUBST(FFTW_SUPPORT)

# liblo options, for OSC server plugin (in src/control/osc)
OSC_SUPPORT=FALSE                         # default
WITH_OSC="false"                          # just for summary display below
AC_ARG_WITH(osc,
   AC_HELP_STRING([--with-osc], [support OSC (Open Sound Control)]),
   [if test "$withval" != "no"; then
      OSC_SUPPORT=TRUE
      WITH_OSC="true"
   fi]
)
AC_SUBST(OSC_SUPPORT)
# LIBGSL options
AC_CHECK_LIB(m,main)
AC_CHECK_LIB(gslcblas,main)
AC_CHECK_LIB(gsl,main)

#AC_SUBST(WITH_VAL, "$withval")    # for debugging this script


# Check for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN

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
         WITH_ALSA="false"
      fi
   fi
fi

# JACK
if test "$JACK_SUPPORT" = "TRUE"; then
   PKG_CHECK_MODULES(JACK,jack)
   AC_SUBST(JACK_CFLAGS)
   AC_SUBST(JACK_LIBS)
fi

# FFTW
# NOTE: If this fails, it could be because user hasn't built FFTW --enable-float
if test "$FFTW_SUPPORT" = "TRUE"; then
   PKG_CHECK_MODULES(FFTW,fftw3f)
   AC_SUBST(FFTW_CFLAGS)
   AC_SUBST(FFTW_LIBS)
fi

# OSC
if test "$OSC_SUPPORT" = "TRUE"; then
   PKG_CHECK_MODULES(LIBLO,liblo)
   AC_SUBST(LIBLO_CFLAGS)
   AC_SUBST(LIBLO_LIBS)
fi


# Write output files -----------------------------------------------------------
AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES(makefile.conf)
AC_CONFIG_FILES(defs.conf)
AC_OUTPUT

# Output summary message
echo
echo "Summary..."
if test "$WITH_64BIT" = "true"; then
echo "  " Building 64-bit binaries
elif test "$WITH_32BIT" = "true"; then
echo "  " Building 32-bit binaries
fi
echo "  " Build with multi-threaded support..................... : $WITH_MULTI_THREAD
echo "  " Build with Perl support............................... : $WITH_PERL
echo "  " Build with Python support............................. : $WITH_PYTHON
echo "  " Build with ALSA support............................... : $WITH_ALSA
echo "  " Build with JACK support............................... : $WITH_JACK
echo "  " Build with NetPlay support............................ : $WITH_NPLAY
echo "  " Build with FFTW support............................... : $WITH_FFTW
echo "  " Build with OSC support................................ : $WITH_OSC
echo

