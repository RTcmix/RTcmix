include ../../makefile.conf

SYSOBJS = $(SRCDIR)/rtcmix/printsf.o $(SRCDIR)/rtcmix/check_byte_order.o \
	$(AUDIODIR)/sndlibsupport.o

SYSLIBS = -lm   # only this, overriding makefile.conf

INCLUDES += -I$(AUDIODIR) -I$(SRCDIR)/sndlib -I$(SRCDIR)/rtcmix

HEAD = sfcreate sfprint sfshrink sfhedit sffixsize

all: $(HEAD)

sfhedit: sfhedit.o $(SYSOBJS)
	$(CC) $(CFLAGS) -o sfhedit sfhedit.o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

sfextend: sfextend.o $(SYSOBJS)
	$(CC) $(CFLAGS) -o sfextend sfextend.o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

sfshrink: sfshrink.o $(SYSOBJS)
	$(CC) $(CFLAGS) -o sfshrink sfshrink.o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

sfcreate: sfcreate.o $(SYSOBJS)
	$(CC) $(CFLAGS) -o sfcreate sfcreate.o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

sfprint: sfprint.o  $(SYSOBJS)
	$(CC) $(CFLAGS) -o sfprint sfprint.o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

sffixsize: sffixsize.o  $(SYSOBJS)
	$(CC) $(CFLAGS) -o sffixsize sffixsize.o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

install: all
	@echo "making install..."	
	@for PGM in $(HEAD); \
	do \
	   ( $(INSTALL) $(CMIXDIR)/utils/head/$$PGM $(DESTDIR); ) \
	done

uninstall:
	@echo "making uninstall..."	
	@for PGM in $(HEAD); \
	do \
	   ( $(RM) $(DESTDIR)/$$PGM; ) \
	done

clean:
	$(RM) *.o $(HEAD)

cleanall: clean uninstall

