# Print embeddable python library, as well as other libs it requires.
# Will prefer static linkage unless invoked with "shared" argument.
# JGG, 8/4/04

import sys, distutils.sysconfig

static_link = 1
nargs = len(sys.argv)
if nargs == 2 and sys.argv[1] == "shared":
   static_link = 0

# Note that this adds libraries we've certainly already linked to.
libs = distutils.sysconfig.get_config_var("LIBS")
libs += " " + distutils.sysconfig.get_config_var("SYSLIBS")

if static_link:
   prefix = distutils.sysconfig.get_config_var("LIBPL")
   pythonlib = distutils.sysconfig.get_config_var("BLDLIBRARY")
   if len(pythonlib) > 0:
      print prefix + '/' + pythonlib, libs
      sys.exit(0)
   # else try shared linkage

linkshared = distutils.sysconfig.get_config_vars("LINKFORSHARED")[0]

# FIXME: Will this sanity test work for all platforms??
# NB: sys.platform can help us if we need to test for platform
if linkshared.find("ython") != -1:
   print linkshared, libs
   sys.exit(0)

print >> sys.stderr, "***ERROR: Can't find a python to embed."
sys.exit(1)

