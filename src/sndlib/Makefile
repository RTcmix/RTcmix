include ../makefile.conf

OBJS = headers.o io.o extra.o

all: sndlib.a

sndlib.a: $(OBJS)
	$(RM) sndlib.a
	$(LD) -r $(OBJS) -o sndlib.a

clean: 
	$(RM) $(OBJS) sndlib.a

distclean:
	$(RM) $(OBJS) sndlib.a

