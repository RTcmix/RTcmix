# NOTE: MIX is built as the main RTcmix executable -- called CMIX, PCMIX,
# or PYCMIX, depending on the parser -- and never as a DSO.

include ../../../makefile.conf

OBJS = MIX.o

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

all: $(OBJS)

standalone: all

$(OBJS): $(INSTRUMENT_H) MIX.h

install: standalone_install

dso_install: standalone_install		# otherwise can't run the dso's

standalone_install: all

uninstall: standalone_uninstall

dso_uninstall: # nothing to do

standalone_uninstall:

clean:
	$(RM) $(OBJS) $(PROGS)

cleanall: clean
	$(RM) depend

