include ../../../makefile.conf

INCLUDES += -I.. -I../../include -I$(INCLUDEDIR)
OBJS = heap.o queue.o rtQueue.o
PROG = rtHeap.o

all: $(PROG)

$(PROG): $(OBJS)
	$(LD) -r $(OBJS) -o $(PROG)

clean:
	$(RM) $(PROG) $(OBJS)

$(OBJS): ../rtdefs.h ../rt.h ../Instrument.h heap.h ../RefCounted.h ../Lockable.h
