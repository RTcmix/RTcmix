include ../makefile.conf

PROGS = cmixplay playraw sfrecord full-duplex-test

SYSOBJS = ../sys/check_byte_order.o
SYSLIBS = -lm   # only this, overriding makefile.conf

ifeq ($(USE_SNDLIB),TRUE)
   SYSOBJS += ../sys/sndlibsupport.o
   SYSLIBS += ../sndlib/sndlib.a
else
endif


all:  $(PROGS)

cmixplay:  play.o $(SYSOBJS)
	$(CC) $(FLAGS) -o cmixplay play.o $(SYSOBJS) $(SYSLIBS)

playraw: playraw.o
	$(CC) -o playraw playraw.o

full-duplex-test: full-duplex-test.o
	$(CC) -o full-duplex-test full-duplex-test.o -lm

sfrecord:  sfrecord.o $(SYSOBJS)
	$(CC) -o sfrecord sfrecord.o $(SYSOBJS) $(SYSLIBS)

install:
	@for PROG in $(PROGS); \
	do \
	( echo "ln -fs $(CMIXDIR)/utils/$$PROG $(BINDIR)"; \
	ln -fs $(CMIXDIR)/utils/$$PROG $(BINDIR); ) \
	done

clean:
	rm -rf *~ *.o core $(PROGS)
	@for PROG in $(PROGS); \
	do \
	( echo "rm -f $(BINDIR)/$$PROG"; \
	rm -f $(BINDIR)/$$PROG; ) \
	done

