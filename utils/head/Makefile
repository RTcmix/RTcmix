include ../makefile.conf

SYSOBJS = ../sys/sfcodes.o ../sys/printsf.o ../sys/wheader.o ../sys/check_byte_order.o
SYSLIBS = -lm   # only this, overriding makefile.conf

ifeq ($(USE_SNDLIB),TRUE)
   SYSOBJS += ../sys/sndlibsupport.o
   SYSLIBS += ../sndlib/sndlib.a
   HEAD = sfcreate sfprint sfshrink sfhedit  # for now; others later
else
   HEAD = sfhedit sfcreate sfprint sfextend sfshrink 
endif

CFLAGS += -Wall


all: $(HEAD)

sfhedit: sfhedit.o $(SYSOBJS)
	cc -o sfhedit sfhedit.o $(SYSOBJS) $(SYSLIBS)

sfextend: sfextend.o $(SYSOBJS)
	cc -o sfextend sfextend.o $(SYSOBJS) $(SYSLIBS)

sfshrink: sfshrink.o $(SYSOBJS)
	cc -o sfshrink sfshrink.o $(SYSOBJS) $(SYSLIBS)

sfcreate: sfcreate.o $(SYSOBJS)
	cc -o sfcreate sfcreate.o $(SYSOBJS) $(SYSLIBS)

sfprint: sfprint.o  $(SYSOBJS)
	cc -o sfprint sfprint.o $(SYSOBJS) $(SYSLIBS)

install:
	@echo "making install..."	
	@for PGM in $(HEAD); \
	do \
	(ln -fs $(CMIXDIR)/head/$$PGM $(DESTDIR); ) \
	done

clean:
	@for PGM in $(HEAD); \
	do \
	( rm -f $(DESTDIR)/$$PGM; ) \
	done
	-rm -f *.o $(HEAD)

