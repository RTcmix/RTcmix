include ../makefile.conf

PROGS = cmixplay playraw sfrecord full-duplex-test

SYSOBJS = ../sys/check_byte_order.o
SYSLIBS = -lm   # only this, overriding makefile.conf

ifeq ($(USE_SNDLIB),TRUE)
   SYSOBJS += ../sys/sndlibsupport.o
   SYSLIBS += ../sndlib/sndlib.a
   PLAY = sndlib_play.o
else
   PLAY = play.o
endif


all: $(PROGS) resample

cmixplay:  $(PLAY) $(SYSOBJS)
	$(CC) -o cmixplay $(PLAY) $(SYSOBJS) $(SYSLIBS)

playraw: playraw.o
	$(CC) -o playraw playraw.o

full-duplex-test: full-duplex-test.o
	$(CC) -o full-duplex-test full-duplex-test.o -lm

sfrecord:  sfrecord.o $(SYSOBJS)
	$(CC) -o sfrecord sfrecord.o $(SYSOBJS) $(SYSLIBS)

resample::
	@echo "making resample ..."
	@cd resample; $(MAKE) all
	@echo "done."; echo ""

install:
	@echo "making install..."	
	@for PROG in $(PROGS); \
	do \
	  ( $(INSTALL) $(CMIXDIR)/utils/$$PROG $(DESTDIR); ) \
	done
	-$(INSTALL) $(CMIXDIR)/utils/resample/resample $(DESTDIR)
	-$(INSTALL) $(CMIXDIR)/utils/cmixplay $(DESTDIR)/play

clean:
	@for PROG in $(PROGS); \
	do \
	  ( $(RM) $(DESTDIR)/$$PROG; ) \
	done
	@cd resample; $(MAKE) clean
	-$(RM) $(DESTDIR)/resample
	-$(RM) $(DESTDIR)/play
	-$(RM) *.o core $(PROGS)

