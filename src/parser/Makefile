include ../../makefile.conf

ifeq ($(PYTHON_SUPPORT), TRUE)
PYTHON_INCLUDE_DIR = `$(PYTHON) ../parser/python/print_python_includedir.py`
endif

.PHONY: minc perl python install-perl install-python clean

minc: parse_with_perl.c rtcmix_parse.h
	$(CC) $(CFLAGS) -c parse_with_minc.c
	(cd minc; $(MAKE) $(MFLAGS) all;)

perl: parse_with_perl.c rtcmix_parse.h
	$(CC) $(CFLAGS) `$(PERL) -MExtUtils::Embed -e ccopts` \
		-DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c parse_with_perl.c
	(cd perl; $(MAKE) $(MFLAGS) all;)

python: parse_with_python.c rtcmix_parse.h
	$(CC) $(CFLAGS) -I$(PYTHON_INCLUDE_DIR) -c parse_with_python.c
	(cd python; $(MAKE) $(MFLAGS) all;)

install-perl:
	(cd perl; $(MAKE) $(MFLAGS) install;)

install-python:
	(cd python; $(MAKE) $(MFLAGS) install;)

clean:
	$(RM) *.o
	(cd minc; $(MAKE) $(MFLAGS) clean;)
	(cd perl; $(MAKE) $(MFLAGS) clean;)
	(cd python; $(MAKE) $(MFLAGS) clean;)

