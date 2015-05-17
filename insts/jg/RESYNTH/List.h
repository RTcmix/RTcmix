/*
 *  Copyright (C) 2012 John Gibson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License, version 2, as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef _LIST_H_
#define _LIST_H_

#include <cstddef>
#include <stdio.h>
#include <limits.h>
//#define NDEBUG
#include <assert.h>

// A light-weight, template singly-linked list class, meant to be used with
// objects that maintain their own <next> link. The type declared for the List
// class must be a pointer to an object that has these public accessor methods:
// 	void next(T elem)		// set next pointer to elem
// 	T next(void)			// get next pointer
// A consequence of this design is that you absolutely may not store more than
// one copy of the same object pointer in a List, or the same object pointer
// in multiple List's!

template <class T> class List {
public:
	List() : _head(NULL), _tail(NULL) {}
	// NB: destruction leaves elements dangling. Call deleteElements first,
	// if desired.

	void pushFront(T elem);	// more efficient than pushBack
	void pushBack(T elem);

	// Remove the first element from the list and return it. NB: does not touch
	// the value of element->next(). There is no popBack, because this would
	// require a full list iteration.
	T popFront(void);

	// Append <list> to this list, which can be empty (but not NULL).
	void appendList(const List *list);

	// Lets us iterate directly, using the embedded object.
	T head(void) const { return _head; }

	T tail(void) const { return _tail; }

	// Remove <elem> from the list without deleting it. Requires a search for
	// <elem>.
	void removeElement(T elem);

	// Remove <elem> from the list, in which <prev> is the node to the left of
	// <elem>, without deleting <elem>. <prev> can be NULL, in which case,
	// <elem> is the head. No error checking is done, so this is fast but
	// possibly dangerous.
	void removeElement(T elem, T prev);

	// Delete all the elements in the list, leaving an empty list.
	void deleteElements(void);

	// Remove all elements from the list, without deleting them or modifying
	// their next links.
	void clear(void);

	// Return the number of elements stored in the list. NB: Must iterate.
	long length(void);

	// Return true if the list contains <elem>.
	bool hasElem(T elem);

	// Print the list. If the list fails the sanity check, attempt to print
	// only if <force> is true, in which case it will print at most maxLen
	// elements before breaking out of the printing loop.
	void dump(bool force=false, int maxLen=INT_MAX);

	// Print warnings about list inconsistency. Return -1 if detected; return
	// 0 if list seems okay. Print even if okay when <printOkay> is true.
	int sanityCheck(bool printOkay=true);

	// Return the number of elements this list shares with <list>.  Optionally
	// print common elements and a summary. This method is very slow, so is only
	// for debugging. It is dangerous for two lists to share elements, because
	// operations on an element can affect its links within another list.
	int haveCommonElems(const List *list, bool print=true);

private:
	T _head;
	T _tail;
};

template <class T> void List<T>::pushFront(T elem)
{
	assert(elem != NULL);
	elem->next(_head);
	_head = elem;
	if (_tail == NULL)	// list was empty
		_tail = elem;
}

template <class T> void List<T>::pushBack(T elem)
{
	assert(elem != NULL);
	if (_tail)
		_tail->next(elem);
	else 		// list is empty
		_head = elem;
	_tail = elem;
	_tail->next(NULL);
}

template <class T> T List<T>::popFront(void)
{
	if (_head == NULL)
		return NULL;
	T elem = _head;
	_head = elem->next();
	if (_head == NULL)
		_tail = NULL;
	return elem;
}

template <class T> void List<T>::appendList(const List *list)
{
	assert(list != NULL);
	if (list->head()) {
		if (_tail)
			_tail->next(list->head());
		else	// this list is empty
			_head = list->head();
		_tail = list->tail();
	}
}

template <class T> void List<T>::removeElement(T elem)
{
	T prev = NULL;
	T p = _head;
	while (p) {
		if (p == elem) {
			removeElement(elem, prev);
			break;
		}
		prev = p;
		p = p->next();
	}
}

template <class T> void List<T>::removeElement(T elem, T prev)
{
	assert(elem != NULL);
	if (elem == _head)
		(void) popFront();
	else {
		assert(prev != NULL);
		if (elem == _tail)
			_tail = prev;
		prev->next(elem->next());
	}
}

template <class T> void List<T>::deleteElements(void)
{
	if (_head) {
		T p = _head;
		while (p) {
			T next = p->next();
			delete p;
			p = next;
		};
		_head = _tail = NULL;
	}
}

template <class T> void List<T>::clear(void)
{
	_head = _tail = NULL;
}

template <class T> long List<T>::length(void)
{
	int count = 0;
	if (_head) {
		T p = _head;
		while (p) {
			p = p->next();
			count++;
		}
	}
	return count;
}

template <class T> bool List<T>::hasElem(T elem)
{
	T p = _head;
	while (p) {
		if (p == elem)
			return true;
		p = p->next();
	}
	return false;
}

template <class T> void List<T>::dump(bool force, int maxLen)
{
	if (sanityCheck(false) == 0 || force) {
		printf("List %p -----------------------------------------------\n", this);
		printf("head=%p, tail=%p\n", _head, _tail);
		T p = _head;
		int count = 0;
		while (p && count < maxLen) {
			printf("node %p, next=%p\n", p, p->next());
			p = p->next();
			count++;
		}
		if (p)
			printf("WARNING: ended printing before list end\n");
		printf("End list %p -------------------------------------------\n", this);
	}
}

template <class T> int List<T>::sanityCheck(bool printOkay)
{
	int status = 0;
	// print out anything that's inconsistent
	if (_head == NULL && _tail != NULL) {
		printf("****** List %p: head is NULL, but tail is not NULL!\n", this);
		status = -1;
	}
	if (_tail == NULL && _head != NULL) {
		printf("****** List %p: tail is NULL, but head is not NULL!\n", this);
		status = -1;
	}
	if (_tail && _tail->next() != NULL) {
		printf("****** List %p: tail->next is not NULL!\n", this);
		status = -1;
	}
	if (_head && (_tail != _head) && (_head->next() == NULL)) {
		printf("****** List %p: head->next is NULL, but tail != head!\n", this);
		status = -1;
	}
	if (status == 0) {
		T p = _head;
		T prev = NULL;
		while (p) {
			prev = p;
			p = p->next();
		}
		if (prev != _tail) {
			printf("****** List %p: tail %p != actual tail %p!\n", this, _tail, prev);
			status = -1;
		}
	}
	if (status == 0 && printOkay)
		printf("List %p seems okay.\n", this);
	return status;
}

template <class T> int List<T>::haveCommonElems(const List *list, bool print)
{
	int count = 0;
	T p = _head;
	while (p) {
		T q = list->head();
		while (q) {
			if (q == p) {
				if (print)
					printf("lists %p and %p share common elem %p\n", this, list, p);
				count++;
			}
			q = q->next();
		}	
		p = p->next();
	}
	if (print) {
		if (count > 0)
			printf("lists %p and %p have %d common elements\n", this, list, count);
		else
			printf("lists %p and %p have no common elements\n", this, list);
	}
	return count;
}

#endif // _LIST_H_
