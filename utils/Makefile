include ../makefile.conf

SYSOBJS = ../sys/check_byte_order.o

ifeq ($(ARCH),LINUX)
   PROGS = cmixplay playraw sfrecord full-duplex-test
   RESAMPLE = resample
   SYSLIBS = -lm   # only this, overriding makefile.conf
endif

ifeq ($(ARCH),SGI)
   PROGS =
   RESAMPLE =
endif

ifeq ($(USE_SNDLIB),TRUE)
   SYSOBJS += ../sys/sndlibsupport.o ../sys/audio_port.o
   PLAY = sndlib_play.o
else
   PLAY = play.o
endif


all: $(PROGS) $(RESAMPLE)

cmixplay:  $(PLAY) $(SYSOBJS)
	$(CC) -o cmixplay $(PLAY) $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

playraw: playraw.o
	$(CC) -o playraw playraw.o

full-duplex-test: full-duplex-test.o
	$(CC) -o full-duplex-test full-duplex-test.o -lm

sfrecord:  sfrecord.o $(SYSOBJS)
	$(CC) -o sfrecord sfrecord.o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

$(RESAMPLE)::
	@echo "making resample ..."
	@cd $(RESAMPLE); $(MAKE) all
	@echo "done."; echo ""

install:
	@echo "making install..."	
ifneq ($(strip $(PROGS)),)       # do only if PROGS is nonempty
	@for PROG in $(PROGS); \
	do \
	  ( $(INSTALL) $(CMIXDIR)/utils/$$PROG $(DESTDIR); ) \
	done
endif
ifneq ($(strip $(RESAMPLE)),)    # do only if RESAMPLE is nonempty
	@$(INSTALL) $(CMIXDIR)/utils/resample/resample $(DESTDIR)
endif
ifneq ($(strip $(PROGS)),)       # do only if PROGS is nonempty
	@$(INSTALL) $(CMIXDIR)/utils/cmixplay $(DESTDIR)/play
endif

clean:
	@for PROG in $(PROGS); \
	do \
	  ( $(RM) $(DESTDIR)/$$PROG; ) \
	done
	@cd $(RESAMPLE); $(MAKE) clean
	@$(RM) $(DESTDIR)/resample
	@$(RM) $(DESTDIR)/play
	@$(RM) *.o core $(PROGS)

