/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include "heap.h"
#include <lock.h>
#include <iostream.h>

heapslot::heapslot()
{
  left=NULL;
  right=NULL;
  parent=NULL;
  inst=NULL;
  chunkStart = 0;
}

heap::heap() : size(0)
{
	pthread_mutex_init(&_mutex, NULL);
}

heap::~heap()
{
	pthread_mutex_destroy(&_mutex);
}

unsigned long heap::getTop()
{
//  Lock topLock(&_mutex);	// This will unlock when it goes out of scope
  return top->chunkStart;
}

void heap::lock()
{
	pthread_mutex_lock(&_mutex);
}

void heap::unlock()
{
	pthread_mutex_unlock(&_mutex);
}

long heap::getSize()
{
  return size;
}

void heap::insert(Instrument *newInst, unsigned long cStart)
{
  heapslot *newHeapElt;
  heapslot *tempHeapElt;
  Instrument *tempInst;
  unsigned long tempChunkStart;

  Lock insertLock(&_mutex);	// This will unlock when it goes out of scope

//  cout << "insert(in):  " << cStart << '\n';

  if (size == 0) {          // SC:  heap is empty ... need to allocate new memory
//    cout << "Heap empty ... initializing\n";
    top = new heapslot;
    leaves.head=NULL;
    leaves.tail=NULL;
    leaves.pushTail(top);
    bot = top;
  }

  newHeapElt = leaves.pop();  // pop bottom most leaf to locate new insertion point

  newHeapElt->left = new heapslot;    // create new slots and put into leaves queue
  newHeapElt->left->parent = newHeapElt;
  newHeapElt->right = new heapslot;
  newHeapElt->right->parent = newHeapElt;
  
  leaves.pushTail(newHeapElt->right);
  leaves.pushTail(newHeapElt->left);

  newHeapElt->inst = newInst;
  newHeapElt->chunkStart = cStart;

  bot = newHeapElt;

  tempHeapElt = newHeapElt;

  // swap instruments based on chunkStart value until heap order is restored
  while (tempHeapElt->parent && 
	 (tempHeapElt->parent->chunkStart > tempHeapElt->chunkStart)) {
    tempInst = tempHeapElt->parent->inst;
    tempChunkStart = tempHeapElt->parent->chunkStart;
    tempHeapElt->parent->inst = tempHeapElt->inst;
    tempHeapElt->parent->chunkStart = tempHeapElt->chunkStart;
    tempHeapElt->inst = tempInst;
    tempHeapElt->chunkStart = tempChunkStart;
    tempHeapElt = tempHeapElt->parent;
  }
  size++;
//  cout << "insert(out): " << tempHeapElt->chunkStart << "\n";
//  cout << "heap::insert ... size = " << size << "\n";
}

Instrument* heap::deleteMin()
{
  int sift;               // flag to tell loop when to stop sifting
  Instrument* retInst;
  Instrument* tempInst;
  heapslot *dead;
  heapslot *oldBot;
  heapslot *tempHeapElt;
  unsigned long tempChunkStart;
  unsigned long retChunkStart;

  Lock deleteLock(&_mutex);	// This will unlock when it goes out of scope

  sift = 1;

  if (size == 0) {  // trap to catch attempt to pop empty heap
    cerr << "ERROR:  heap underflow\n";
    size=0;
    return NULL;
  }

  retInst = top->inst;    // return instrument is the top instrument
  retChunkStart = top->chunkStart;

  top->inst = bot->inst;  // replace top instrument with bottom instrument
  top->chunkStart = bot->chunkStart;

                          // for filtering down (later)
  oldBot = bot;           // store old "bottom"

  bot->left = NULL;
  bot->right = NULL;
  bot->inst = NULL;
  bot->chunkStart = 0;

  if (size > 1) {             // SC: only want to pop one item if size == 1
    dead = leaves.popTail();  // trim last (two) element(s) appended to leaves
    delete dead;              // and free up memory
  }
  dead = leaves.popTail();    
  delete dead;

  bot = leaves.tail->heap->parent;  // reset bottom
  
  leaves.push(oldBot);        // push old bottom onto top of leaves

  tempHeapElt = top;          // place new element onto top of heap

  // filter top item down through heap until heap order is restored

  while ((tempHeapElt->inst && 
	(tempHeapElt->left->inst || tempHeapElt->right->inst) && sift)) {
    if (!tempHeapElt->left->inst) {
      if (tempHeapElt->right->chunkStart < tempHeapElt->chunkStart) {
	tempInst = tempHeapElt->right->inst;
	tempChunkStart = tempHeapElt->right->chunkStart;
	tempHeapElt->right->inst = tempHeapElt->inst;
	tempHeapElt->right->chunkStart = tempHeapElt->chunkStart;
	tempHeapElt->inst = tempInst;
	tempHeapElt->chunkStart = tempChunkStart;
	tempHeapElt = tempHeapElt->right;
      }
      else
	sift = 0;
    }
    else if (!tempHeapElt->right->inst) {
      if (tempHeapElt->left->chunkStart < tempHeapElt->chunkStart) {
	tempInst = tempHeapElt->left->inst;
	tempChunkStart = tempHeapElt->left->chunkStart;
	tempHeapElt->left->inst = tempHeapElt->inst;
	tempHeapElt->left->chunkStart = tempHeapElt->chunkStart;
	tempHeapElt->inst = tempInst;
	tempHeapElt->chunkStart = tempChunkStart;
	tempHeapElt = tempHeapElt->left;
      }
      else
	sift = 0;
    }
    if (tempHeapElt->left->inst && tempHeapElt->right->inst) {
      if(tempHeapElt->left->chunkStart < tempHeapElt->right->chunkStart) {
	if (tempHeapElt->left->chunkStart < tempHeapElt->chunkStart) {
	tempInst = tempHeapElt->left->inst;
	tempChunkStart = tempHeapElt->left->chunkStart;
	tempHeapElt->left->inst = tempHeapElt->inst;
	tempHeapElt->left->chunkStart = tempHeapElt->chunkStart;
	tempHeapElt->inst = tempInst;
	tempHeapElt->chunkStart = tempChunkStart;
	tempHeapElt = tempHeapElt->left;
	}
	else
	  sift = 0;
      }
      else if(tempHeapElt->left->chunkStart >= tempHeapElt->right->chunkStart)
	if (tempHeapElt->right->chunkStart < tempHeapElt->chunkStart) {
	tempInst = tempHeapElt->right->inst;
	tempChunkStart = tempHeapElt->right->chunkStart;
	tempHeapElt->right->inst = tempHeapElt->inst;
	tempHeapElt->right->chunkStart = tempHeapElt->chunkStart;
	tempHeapElt->inst = tempInst;
	tempHeapElt->chunkStart = tempChunkStart;
	tempHeapElt = tempHeapElt->right;
	}
	else
	  sift = 0;
    }
  }
//  cout << "deleteMin(): " << retChunkStart << "\n";
  size--;
  return retInst;
}

void heapslot::dump(int indent)
{
  int i;

  if (this->inst == NULL)
    return;
  
  if (left->inst)
    left->dump(indent+1);
  
  for (i=0; i<indent; i++)
    cout << "    ";
  cout << chunkStart << "\n";
  
  if (right->inst)
    right->dump(indent+1);
  return;
}












