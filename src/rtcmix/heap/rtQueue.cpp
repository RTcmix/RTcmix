/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <iostream.h>
#include "heap.h"
#include "../../H/dbug.h"
#include "../rtdefs.h"
#include <globals.h>
#include <Instrument.h>

// #define DBUG

RTQueue::RTQueue()
{
  head=NULL;
  tail=NULL;
  size=0;
}

// Return the number of elements on the RTQueue

int RTQueue::getSize()
{
  return size;
}

// Return the starting sample chunk of the top Instrument

int RTQueue::nextChunk()
{
  return head->chunkstart;
}

// Push an element to end of the RTQueue

void RTQueue::push(Instrument *newInst, unsigned long new_chunkstart)
{
  int i;
  long diff;
  rtQElt *newElt;  // create new rtQElt
  rtQElt *tempElt; // BGG: for queue insertion
  newElt = new rtQElt;
  newElt->Inst = newInst;
  newElt->Inst->Ref();
  newElt->chunkstart = new_chunkstart; 
  newElt->next = NULL;
  newElt->prev = NULL;

  if (head == NULL)  // if first item on RTQueue
    head = tail = newElt;
  else if(tail->chunkstart <= newElt->chunkstart) {
    // append to the end of the RTQueue
#ifdef DBUG
	cout << "Queueing at end\n";
#endif
    tail->next = newElt;
    newElt->prev = tail;
    tail = newElt;
  }
  else { // BGG: we have to insert this one
#ifdef DBUG
    cout << "RTQueue::push():  scanning ...\n";
    cout << "tail->chunkstart = " << tail->chunkstart << endl;
    cout << "newElt->chunkstart = " << newElt->chunkstart << endl;
    cout << "Queue size = " << size << endl;
#endif
    tempElt = tail;
    i = 1;
#ifdef DBUG    
    cout << "STARTING SCAN\n";
#endif
    while((tempElt->chunkstart > newElt->chunkstart) && (tempElt->prev) && (i < size)){
      if (!tempElt->prev) { // BGG: we're at the head
	cout << "We're at the head\n";
	break;
      }
#ifdef DBUG
      cout << "i = " << i << endl;
      cout << "tempElt = " << tempElt << endl;
      cout << "head = " << head << endl;
      cout << "head->chunkstart = " << head->chunkstart << endl;
      cout << "tempElt->chunkstart = " << tempElt->chunkstart << endl;
      cout << "newElt->chunkstart = " << newElt->chunkstart << endl;
      diff = (newElt->chunkstart - tempElt->chunkstart);
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
    cout << "head->chunkstart = " << head->chunkstart << endl;
    cout << "tempElt->chunkstart = " << tempElt->chunkstart << endl;
    cout << "newElt->chunkstart = " << newElt->chunkstart << endl;
    diff = (newElt->chunkstart - tempElt->chunkstart);
    cout << "diff = " << diff << endl;
    cout << "tempElt->prev = " << tempElt->prev << endl;

    if (diff > NCHANS * RTBUFSAMPS) {
      cerr << "SCHEDULING INCONSISTENCY!\n";
      cerr << "newElt->chunkstart = " << newElt->chunkstart << endl;
      cout << "tempElt->chunkstart = " << tempElt->chunkstart << endl;
      diff = (newElt->chunkstart - tempElt->chunkstart);
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

// Pop an element of the top of the RTQueue

Instrument *RTQueue::pop() 
{
  rtQElt *tQelt;
  Instrument *retInst;
  tQelt = head;
  if (!head) {
    cerr << "ERROR: attempt to pop empty RTQueue\n";
    return NULL;
  }
  retInst = head->Inst;
  head = head->next;
  delete tQelt;
  size--;
  retInst->Unref();
  return retInst;
}





