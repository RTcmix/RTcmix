#ifndef _HEAP_H_
#define _HEAP_H_ 1

#include <Lockable.h>

class Instrument;

// class for heap element structure

class heapslot {
public:
  unsigned long chunkStart; // start samp for chunk
  Instrument *inst;
  heapslot *left, *right, *parent;

  heapslot();
  void dump(int);
};

// class for queue used to hold heap end

class qElt {
  friend class queue;
  friend class heap;
  qElt *next;
  qElt *prev;
  heapslot *heap;
};

// class for main queue structure

class queue {
private:
  friend class heap;
  qElt *head;
  qElt *tail;
public:
  queue();
  ~queue();
  void pushTail(heapslot*);
  void push(heapslot*);
  heapslot *pop();
  heapslot *popTail();
  void print();
};

// class for main heap structure

class heap : public Lockable {
private:
  queue leaves;  // queue used to hold next insertion point
public:
  heapslot* bot;
  heapslot* top;
  heap();
  ~heap();
  unsigned long getTop();
  long getSize();
  void insert(Instrument*, unsigned long chunkStart);
  Instrument *deleteMin(unsigned long maxChunkStart, unsigned long *pChunkStart);
  void dump(heapslot*,int);
  long size;
};

// class for queue used to hold Instruments

class rtQElt {
public:
  rtQElt(Instrument *inst, unsigned long start);
  ~rtQElt();
private:
  friend class RTQueue;
  rtQElt *next;
  rtQElt *prev;
  Instrument *Inst;
  unsigned long chunkstart;
};

// class for main queue structure

class RTQueue {
private:
  rtQElt *head;
  rtQElt *tail;
  int size;
public:
  RTQueue();
  void push(Instrument*, unsigned long);
  Instrument *pop();
  int nextChunk();
  int getSize();
  void print();  // For debugging
};


#endif /* _HEAP_H_ */

