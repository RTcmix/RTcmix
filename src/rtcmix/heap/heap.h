#ifndef _HEAP_H_
#define _HEAP_H_ 1

#include "../Instrument.h"

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
  void pushTail(heapslot*);
  void push(heapslot*);
  heapslot *pop();
  heapslot *popTail();
  void print();
};

// class for main heap structure

class heap {
private:
  queue leaves;  // queue used to hold next insertion point
public:
  heapslot* bot;
  heapslot* top;
  heap();
  unsigned long getTop();
  long getSize();
  void insert(Instrument*, unsigned long chunkStart);
  Instrument *deleteMin();
  void dump(heapslot*,int);
  long size;
};

// class for queue used to hold Instruments

class rtQElt {
  friend class rtQueue;
  rtQElt *next;
  rtQElt *prev;
  Instrument *Inst;
  unsigned long chunkstart;
};

// class for main queue structure

class rtQueue {
private:
  rtQElt *head;
  rtQElt *tail;
  int size;
public:
  rtQueue();
  void push(Instrument*, unsigned long);
  Instrument *pop();
  int nextChunk();
  int getSize();
  void print();  // For debugging
};


#endif /* _HEAP_H_ */

