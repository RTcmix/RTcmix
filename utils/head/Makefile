include ../makefile.conf

SYSOBJS = ../sys/printsf.o ../sys/check_byte_order.o ../sys/sndlibsupport.o

ifeq ($(ARCH),LINUX)
   SYSLIBS = -lm   # only this, overriding makefile.conf
endif
ifeq ($(ARCH),FREEBSD)
   SYSLIBS = -lm   # only this, overriding makefile.conf
endif

HEAD = sfcreate sfprint sfshrink sfhedit sffixsize

all: $(HEAD)

sfhedit: sfhedit.o $(SYSOBJS)
	$(CC) -o sfhedit sfhedit.o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

sfextend: sfextend.o $(SYSOBJS)
	$(CC) -o sfextend sfextend.o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

sfshrink: sfshrink.o $(SYSOBJS)
	$(CC) -o sfshrink sfshrink.o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

sfcreate: sfcreate.o $(SYSOBJS)
	$(CC) -o sfcreate sfcreate.o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

sfprint: sfprint.o  $(SYSOBJS)
	$(CC) -o sfprint sfprint.o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

sffixsize: sffixsize.o  $(SYSOBJS)
	$(CC) -o sffixsize sffixsize.o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

install:
	@echo "making install..."	
	@for PGM in $(HEAD); \
	do \
	   ( $(INSTALL) $(CMIXDIR)/head/$$PGM $(DESTDIR); ) \
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

