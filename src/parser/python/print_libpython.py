# Print embeddable python library, as well as other libs it requires.
# Will prefer static linkage unless invoked with "shared" argument.
# JGG, 8/4/04

import sys, os.path, distutils.sysconfig

static_link = 1
nargs = len(sys.argv)
if nargs == 2 and sys.argv[1] == "shared":
   static_link = 0

# Note that this adds libraries we've certainly already linked to.
libs = distutils.sysconfig.get_config_var("LIBS")
libs += " " + distutils.sysconfig.get_config_var("SYSLIBS")

prefix = distutils.sysconfig.get_config_var("LIBPL")

if static_link:
   pythonlib = distutils.sysconfig.get_config_var("LIBRARY")
   if len(pythonlib) > 0:
      plib = prefix + '/' + pythonlib
      # Must see if file exists, because it doesn't in Jaguar!
      if os.path.exists(plib):
         print plib, libs
         sys.exit(0)
   # else try shared linkage...

linkshared = distutils.sysconfig.get_config_vars("LINKFORSHARED")[0]

# On OS X 10.3, linkshared will include "-framework Python" (unless the
# installation is broken).  That's all we need.  On Linux, linkshared
# does not include the name of the shared library, so we add it below.

if sys.platform.find("darwin") != -1:     # if it's OS X
   if linkshared.find("Python") != -1:    # includes "-framework Python"
      print linkshared, libs
      sys.exit(0)
else:
   pythonlib = distutils.sysconfig.get_config_var("LDLIBRARY")
   if len(pythonlib) > 0:
      if pythonlib.endswith(".so"):       # sanity check
         plib = prefix + '/' + pythonlib
         if os.path.exists(plib):
            print linkshared, plib, libs
            sys.exit(0)

print >> sys.stderr, "***ERROR: Can't find a python library to embed."
sys.exit(1)

