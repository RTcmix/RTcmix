include ../../makefile.conf

#OBJS = heap.o queue.o heapify.o heapSched.o rtQueue.o
OBJS = heap.o queue.o rtQueue.o
PROG = rtHeap.o

all: $(PROG)

$(PROG): $(OBJS)
	$(LD) -r $(OBJS) -o $(PROG)

test: testmain.o heap.o queue.o rtQueue.o osx_workaround.o
	$(CXX) -o test osx_workaround.o testmain.o heap.o queue.o rtQueue.o

clean:
	$(RM) $(PROG) $(OBJS) test testmain.o

$(OBJS): ../rtdefs.h ../rt.h ../Instrument.h heap.h
