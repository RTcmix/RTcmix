include ../makefile.conf

PROGS = cmixplay playraw sfrecord full-duplex-test

all:  $(PROGS)

cmixplay:  play.o ../sys/check_byte_order.o
	$(CC) $(FLAGS) -o cmixplay play.o ../sys/check_byte_order.o -lm

playraw: playraw.o
	$(CC) -o playraw playraw.o

full-duplex-test: full-duplex-test.o
	$(CC) -o full-duplex-test full-duplex-test.o -lm

sfrecord:  sfrecord.o ../sys/check_byte_order.o
	$(CC) -o sfrecord sfrecord.o ../sys/check_byte_order.o

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

