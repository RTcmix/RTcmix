include ../../makefile.conf

DIRS = minc
SRCS = parse_with_minc.c
OBJS = parse_with_minc.o

ifeq ($(PERL_SUPPORT), TRUE)
	DIRS += perl
	OBJS += parse_with_perl.o
	SRCS += parse_with_perl.c
endif
ifeq ($(PYTHON_SUPPORT), TRUE)
	DIRS += python
	OBJS += parse_with_python.o
	SRCS += parse_with_python.c
   PYTHON_INCLUDE_DIR = `$(PYTHON) python/print_python_includedir.py`
   ifeq ($(PYTHON_EXT_STATIC_LINK), TRUE)
      PYDEF = -DPYEXT_INIT
   endif
endif

all: $(DIRS) $(OBJS)
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) all; ) \
	done

parse_with_minc.o: parse_with_minc.c rtcmix_parse.h
	$(CC) $(CFLAGS) -c parse_with_minc.c

parse_with_perl.o: parse_with_perl.c rtcmix_parse.h
	$(CC) $(CFLAGS) -c parse_with_perl.c

ifeq ($(ARCH),MACOSX)
ifeq ($(BUILD_ARCH),ppc)
	$(CC) $(CFLAGS) `$(PERL) -MExtUtils::Embed -e ccopts | sed 's/-arch i386//' | sed 's/-arch x86_64//'` \
		-DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c parse_with_perl.c
endif
ifeq ($(BUILD_ARCH),i386)
	$(CC) $(CFLAGS) `$(PERL) -MExtUtils::Embed -e ccopts | sed 's/-arch x86_64//' | sed 's/-arch ppc//'` \
		-DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c parse_with_perl.c
endif
ifeq ($(BUILD_ARCH),x86_64)
	$(CC) $(CFLAGS) `$(PERL) -MExtUtils::Embed -e ccopts | sed 's/-arch i386//' | sed 's/-arch ppc//'` \
		-DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c parse_with_perl.c
endif
else
	$(CC) $(CFLAGS) `$(PERL) -MExtUtils::Embed -e ccopts` \
		-DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c parse_with_perl.c
endif

parse_with_python.o: parse_with_python.c rtcmix_parse.h
	$(CC) $(CFLAGS) -I$(PYTHON_INCLUDE_DIR) $(PYDEF) -c parse_with_python.c

.PHONY: install clean

install:
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) install; ) \
	done

uninstall:
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) uninstall; ) \
	done

clean:
	$(RM) *.o depend core
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) clean; ) \
	done

cleanall:
	$(RM) *.o depend core
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) cleanall; ) \
	done

depend:
ifeq ($(ARCH),LINUX)
	-$(SHELL) -ec '$(CC) -M $(CFLAGS) $(SRCS)' > depend
endif
ifeq ($(ARCH),SGI)
# Note: -MM is a gcc extension. If no gcc, use -M.
# # Unfortunately, this doesn't work...   -JGG
# #   $(SHELL) -ec '$(CC) -M $(CFLAGS) $(SRCS)' > depend
endif

