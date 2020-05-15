include ../../makefile.conf

OBJS = headers.o io.o extra.o

all: sndlib.a

ifeq ($(BUILDTYPE), STANDALONE)
sndlib.a: $(OBJS)
	$(RM) sndlib.a
	$(LD) -r $(OBJS) -o sndlib.a
else
sndlib.a: $(OBJS)
	$(RM) $@
	ar q $@ $(OBJS)
	$(RANLIB) $@
endif

install: all
uninstall:

clean: 
	$(RM) $(OBJS) sndlib.a

cleanall: clean

distclean: clean
	$(RM) depend

