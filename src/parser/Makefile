include ../../makefile.conf

DIRS = minc
SRCS = parse_with_minc.cpp
OBJS = parse_with_minc.o
INCLUDES += -I../rtcmix

ifeq ($(PERL_SUPPORT), TRUE)
  DIRS += perl
  OBJS += parse_with_perl.o
  SRCS += parse_with_perl.c
  ifeq ($(ARCH), MACOSX)
    PERL_CCFLAGS = `sh perl/print_perl_includes.sh $(PERL) $(BUILD_ARCH)`
  else
    PERL_CCFLAGS = `$(PERL) -MExtUtils::Embed -e ccopts`
  endif
endif
ifeq ($(PYTHON_SUPPORT), TRUE)
  DIRS += python
  OBJS += parse_with_python.o
  SRCS += parse_with_python.c
  PYTHON_CCFLAGS = -I`$(PYTHON) python/print_python_includedir.py`
  ifeq ($(PYTHON_EXT_STATIC_LINK), TRUE)
    PYDEF = -DPYEXT_INIT
  endif
endif

all: $(DIRS) $(OBJS)

$(DIRS)::
	@echo "making all in parser..."
	$(MAKE) $(MFLAGS) -C $@

parse_with_minc.o: parse_with_minc.cpp rtcmix_parse.h
	$(CXX) $(CXXFLAGS) -c parse_with_minc.cpp

parse_with_perl.o: parse_with_perl.c rtcmix_parse.h
	$(CC) $(CFLAGS) $(PERL_CCFLAGS) -DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c parse_with_perl.c

parse_with_python.o: parse_with_python.c rtcmix_parse.h
	$(CC) $(CFLAGS) $(PYTHON_CCFLAGS) $(PYDEF) -c parse_with_python.c

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
	$(RM) *.o core
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
ifeq ($(ARCH),SGI)
# Note: -MM is a gcc extension. If no gcc, use -M.
# # Unfortunately, this doesn't work...   -JGG
# #   $(SHELL) -ec '$(CC) -M $(CFLAGS) $(SRCS)' > depend
else
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) depend; ) \
	done
	-$(SHELL) -ec '$(CC) -M $(CFLAGS) $(PERL_CCFLAGS) $(PYTHON_CCFLAGS) $(SRCS)' > depend
endif

