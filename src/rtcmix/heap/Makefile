include ../../makefile.conf

OBJS = heap.o queue.o heapify.o heapSched.o rtQueue.o
PROG = rtHeap.o

all: $(PROG)

$(PROG): $(OBJS)
	$(LD) -r $(OBJS) -o $(PROG)

clean:
	rm -f $(PROG) $(OBJS)

$(OBJS): ../rtdefs.h ../rt.h ../Instrument.h heap.h
