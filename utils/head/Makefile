include ../makefile.conf

SYSOBJS = ../sys/sfcodes.o ../sys/printsf.o ../sys/wheader.o ../sys/check_byte_order.o

ifeq ($(ARCH),LINUX)
   SYSLIBS = -lm   # only this, overriding makefile.conf
endif

ifeq ($(USE_SNDLIB),TRUE)
   SYSOBJS += ../sys/sndlibsupport.o
   HEAD = sfcreate sfprint sfshrink sfhedit sffixsize
else
   HEAD = sfhedit sfcreate sfprint sfextend sfshrink 
endif

#CFLAGS += -Wall


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

clean:
	@for PGM in $(HEAD); \
	do \
	   ( $(RM) $(DESTDIR)/$$PGM; ) \
	done
	@$(RM) *.o $(HEAD)

