include ../../makefile.conf

ifeq ($(PYTHON_SUPPORT), TRUE)
   PYTHON_INCLUDE_DIR = `$(PYTHON) python/print_python_includedir.py`
   ifeq ($(PYTHON_EXT_LINK), TRUE)
      PYDEF = -DPYEXT_INIT
   endif
endif

.PHONY: minc perl python install-perl install-python clean

DIRS = minc
SRCS = parse_with_minc.c
OBJS = parse_with_minc.o

ifeq ($(PERL_SUPPORT), TRUE)
	DIRS += perl
	OBJS += parse_with_perl.o
	SRCS += parse_with_perl.c
	CFLAGS += `$(PERL) -MExtUtils::Embed -e ccopts` -DSHAREDLIBDIR=\"$(LIBDESTDIR)\"
endif
ifeq ($(PYTHON_SUPPORT), TRUE)
	DIRS += python
	INCLUDES += -I$(PYTHON_INCLUDE_DIR)
	CLFAGS += $(PYDEF)
	OBJS += parse_with_python.o
	SRCS += parse_with_python.c
endif


all: $(DIRS) $(OBJS)
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) all; ) \
	done

install: all
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) install; ) \
	done

clean:
	$(RM) *.o depend core
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) clean; ) \
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
	
