/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <iostream.h>
#include "heap.h"
#include "../../H/dbug.h"
#include "../rtdefs.h"

rtQueue::rtQueue()
{
  head=NULL;
  tail=NULL;
  size=0;
}

// Return the number of elements on the rtQueue

int rtQueue::getSize()
{
  return size;
}

// Return the starting sample chunk of the top Instrument

int rtQueue::nextChunk()
{
  return head->Inst->chunkstart;
}

// Push an element to end of the rtQueue

void rtQueue::push(Instrument *newInst)
{
  int i;
  long diff;
  rtQElt *newElt;  // create new rtQElt
  rtQElt *tempElt; // BGG: for queue insertion
  newElt = new rtQElt;
  newElt->Inst = newInst;
  newElt->next = NULL;
  newElt->prev = NULL;
  

  if (head == NULL)  // if first item on rtQueue
    head = tail = newElt;
  else if(tail->Inst->chunkstart <= newElt->Inst->chunkstart) {
    // append to the end of the rtQueue
    tail->next = newElt;
    newElt->prev = tail;
    tail = newElt;
  }
  else { // BGG: we have to insert this one
#ifdef DBUG
    cout << "rtQueue::push():  scanning ...\n";
    cout << "tail->Inst->chunkstart = " << tail->Inst->chunkstart << endl;
    cout << "newElt->Inst->chunkstart = " << newElt->Inst->chunkstart << endl;
    cout << "Queue size = " << size << endl;
#endif
    tempElt = tail;
    i = 1;
#ifdef DBUG    
    cout << "STARTING SCAN\n";
#endif
    while((tempElt->Inst->chunkstart > newElt->Inst->chunkstart) && (tempElt->prev) && (i < size)){
      if (!tempElt->prev) { // BGG: we're at the head
	cout << "We're at the head\n";
	break;
      }
#ifdef DBUG
      cout << "i = " << i << endl;
      cout << "tempElt = " << tempElt << endl;
      cout << "head = " << head << endl;
      cout << "head->Inst->chunkstart = " << head->Inst->chunkstart << endl;
      cout << "tempElt->Inst->chunkstart = " << tempElt->Inst->chunkstart << endl;
      cout << "newElt->Inst->chunkstart = " << newElt->Inst->chunkstart << endl;
      diff = (newElt->Inst->chunkstart - tempElt->Inst->chunkstart);
      cout << "diff = " << diff << endl;
      cout << "tempElt->prev = " << tempElt->prev << endl;
#endif
      tempElt = tempElt->prev;
      i++;
    }
#ifdef DBUG
    cout << "DONE SCANNING\n";
    cout << "i = " << i << endl;
    cout << "tempElt = " << tempElt << endl;
    cout << "head = " << head << endl;
    cout << "head->Inst->chunkstart = " << head->Inst->chunkstart << endl;
    cout << "tempElt->Inst->chunkstart = " << tempElt->Inst->chunkstart << endl;
    cout << "newElt->Inst->chunkstart = " << newElt->Inst->chunkstart << endl;
    diff = (newElt->Inst->chunkstart - tempElt->Inst->chunkstart);
    cout << "diff = " << diff << endl;
    cout << "tempElt->prev = " << tempElt->prev << endl;

    if (diff > MAXBUF) {  // 128 was MAXBUF in my testing
      cerr << "SCHEDULING INCONSISTENCY!\n";
      cerr << "newElt->Inst->chunkstart = " << newElt->Inst->chunkstart << endl;
      cout << "tempElt->Inst->chunkstart = " << tempElt->Inst->chunkstart << endl;
      diff = (newElt->Inst->chunkstart - tempElt->Inst->chunkstart);
      cout << "diff = " << diff << endl;
      exit(1);
    }
#endif

    if ((tempElt->prev) && (i < size)) {  // BGG: "normal" insertion
#ifdef DBUG
      cout << "Normal insertion\n";
#endif
      newElt->next = tempElt->next;
      newElt->prev = tempElt;
      (tempElt->next)->prev = newElt;
      tempElt->next = newElt;
    }
    else { // BGG: put at the head
#ifdef DBUG
      cout << "Put it at the head\n";
#endif
      newElt->next = head;
      newElt->prev = NULL;
      // BGG:  I thought I would have to do the following line here,
      //	but experience shows that it dumps core and does bad things
      //		(tempElt->next)->prev = newElt;
      head = newElt;
    }
  }
  size++;
}

// Pop an element of the top of the rtQueue

Instrument *rtQueue::pop() 
{
  rtQElt *tQelt;
  Instrument *retInst;
  tQelt = head;
  if (!head) {
    cerr << "ERROR: attempt to pop empty rtQueue\n";
    return NULL;
  }
  retInst = head->Inst;
  head = head->next;
  delete tQelt;
  size--;
  return retInst;

}





