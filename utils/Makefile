include ../makefile.conf

PLAY = cmixplay

ifeq ($(ARCH),LINUX)
   PROGS = $(PLAY) playraw full-duplex-test
   RESAMPLE = resample
   SYSLIBS = -lm   # only this, overriding makefile.conf
endif

# nothing here supported under Irix yet
ifeq ($(ARCH),SGI)
   PROGS =
   RESAMPLE =
endif

SYSOBJS = ../sys/check_byte_order.o ../sys/sndlibsupport.o ../sys/audio_port.o


all: $(PROGS) $(RESAMPLE)

$(PLAY):  $(PLAY).o $(SYSOBJS)
	$(CC) -o cmixplay $(SOUND_DRIVER) $(PLAY).o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

$(PLAY).o:  $(PLAY).c ../H/audio_port.h ../H/sndlibsupport.h
	$(CC) $(CFLAGS) $(SOUND_DRIVER) -c $< -o $@

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
ifneq ($(strip $(PLAY)),)        # do only if PLAY is nonempty
	@cd $(DESTDIR); ln -fs $(PLAY) play
endif

clean:
ifneq ($(strip $(PROGS)),)       # do only if PROGS is nonempty
	@for PROG in $(PROGS); \
	do \
	  ( $(RM) $(DESTDIR)/$$PROG; ) \
	done
	@$(RM) $(DESTDIR)/play
endif
ifneq ($(strip $(RESAMPLE)),)    # do only if RESAMPLE is nonempty
	@cd $(RESAMPLE); $(MAKE) clean
endif
	@$(RM) *.o core $(PROGS)

