include ../makefile.conf

ifeq ($(USE_SNDLIB),TRUE)
   SFSTUFF = ../sys/sndlibsupport.o
   SYSLIBS += ../sndlib/sndlib.a
   HEAD = sfcreate sfprint   # for now; others later
else
   SFSTUFF = ../sys/wheader.o ../sys/sfcodes.o ../sys/printsf.o \
	../sys/check_byte_order.o
   HEAD = sfhedit sfcreate sfprint sfextend sfshrink 
endif


all: $(HEAD)

sfhedit: sfhedit.o $(SFSTUFF)
	cc -o sfhedit sfhedit.o $(SFSTUFF) $(SYSLIBS)

sfextend: sfextend.o $(SFSTUFF)
	cc -o sfextend sfextend.o $(SFSTUFF) $(SYSLIBS)

sfshrink: sfshrink.o $(SFSTUFF)
	cc -o sfshrink sfshrink.o $(SFSTUFF) $(SYSLIBS)

sfcreate: sfcreate.o $(SFSTUFF)
	cc -o sfcreate sfcreate.o $(SFSTUFF) $(SYSLIBS)

sfprint: sfprint.o  $(SFSTUFF)
	cc -o sfprint sfprint.o $(SFSTUFF) $(SYSLIBS)

install:
	@echo "making install..."	
	@for PGM in $(HEAD); \
	do \
	(ln -fs $(CMIXDIR)/head/$$PGM $(DESTDIR); ) \
	done

clean:
	-rm -f *.o $(HEAD)

