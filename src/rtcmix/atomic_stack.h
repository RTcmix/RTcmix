#ifndef __CAAtomicStack_h__
#define __CAAtomicStack_h__

#if defined(__cpp_lib_atomic) || __cplusplus >= 199711L
#define USE_ATOMIC 1
#include <atomic>
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#define USE_ATOMIC 1
#include <atomic>
#else
#define USE_ATOMIC 0
#endif

#ifdef MACOSX
#include <libkern/OSAtomic.h>
#elif defined(LINUX)
template <class T>
static bool	compare_and_swap(T *oldvalue, T *newvalue, T **pvalue);
#endif

#ifndef NULL
#define NULL 0
#endif

//  linked list LIFO or FIFO (pop_all_reversed) stack, elements are pushed and popped atomically
//  class T must implement T *& next().
template <class T>
class TAtomicStack {
private:
    bool	compare_and_swap(T *oldvalue, T *newvalue, T **pvalue)
    {
#if USE_ATOMIC
        return std::atomic_compare_exchange_strong(
                                oldvalue,
                                newvalue,
                                pvalue);
#elif defined(MACOSX)
        return ::OSAtomicCompareAndSwap64Barrier(int64_t(oldvalue), int64_t(newvalue), (int64_t *)pvalue);
#else
		return __sync_bool_compare_and_swap(pvalue, oldvalue, newvalue);
#endif
    }
public:
	TAtomicStack() : mHead(NULL) { }
	
	// non-atomic routines, for use when initializing/deinitializing, operate NON-atomically
	void	push_NA(T *item)
	{
		item->next() = mHead;
		mHead = item;
	}
	
	T *		pop_NA()
	{
		T *result = mHead;
		if (result)
			mHead = result->next();
		return result;
	}
	
	bool	empty() { return mHead == NULL; }
	
	T *		head() { return mHead; }
	
	// atomic routines
	void	push_atomic(T *item)
	{
		T *head_;
		do {
			head_ = mHead;
			item->next() = head_;
		} while (!compare_and_swap(head_, item, &mHead));
	}
	
	void	push_multiple_atomic(T *item)
	// pushes entire linked list headed by item
	{
		T *head_, *p = item, *tail;
		// find the last one -- when done, it will be linked to head
		do {
			tail = p;
			p = p->next();
		} while (p);
		do {
			head_ = mHead;
			tail->next() = head_;
		} while (!compare_and_swap(head_, item, &mHead));
	}
	
	T *		pop_atomic_single_reader()
	// this may only be used when only one thread may potentially pop from the stack.
	// if multiple threads may pop, this suffers from the ABA problem.
	// <rdar://problem/4606346> TAtomicStack suffers from the ABA problem
	{
		T *result;
		do {
			if ((result = mHead) == NULL)
				break;
		} while (!compare_and_swap(result, result->next(), &mHead));
		return result;
	}
	
	T *		pop_atomic()
	// This is inefficient for large linked lists.
	// prefer pop_all() to a series of calls to pop_atomic.
	// push_multiple_atomic has to traverse the entire list.
	{
		T *result = pop_all();
		if (result) {
			T *next = result->next();
			if (next)
				// push all the remaining items back onto the stack
				push_multiple_atomic(next);
		}
		return result;
	}
	
	T *		pop_all()
	{
		T *result;
		do {
			if ((result = mHead) == NULL)
				break;
		} while (!compare_and_swap(result, NULL, &mHead));
		return result;
	}
	
	T*		pop_all_reversed()
	{
		TAtomicStack<T> reversed;
		T *p = pop_all(), *next;
		while (p != NULL) {
			next = p->next();
			reversed.push_NA(p);
			p = next;
		}
		return reversed.mHead;
	}
protected:
	T *		mHead;
};

#ifdef MACOSX

#include <libkern/OSAtomic.h>

class CAAtomicStack {
public:
	CAAtomicStack(size_t nextPtrOffset) : mNextPtrOffset(nextPtrOffset) {
		/*OSQueueHead h = OS_ATOMIC_QUEUE_INIT; mHead = h;*/
		mHead.opaque1 = 0; mHead.opaque2 = 0;
	}
	// a subset of the above
	void	push_atomic(void *p) { OSAtomicEnqueue(&mHead, p, mNextPtrOffset); }
	void *	pop_atomic() { return OSAtomicDequeue(&mHead, mNextPtrOffset); }

private:
	OSQueueHead		mHead;
	size_t			mNextPtrOffset;
};

// a more efficient subset of TAtomicStack using OSQueue.
template <class T>
class TAtomicStack2 {
public:
	TAtomicStack2() {
		/*OSQueueHead h = OS_ATOMIC_QUEUE_INIT; mHead = h;*/
		mHead.opaque1 = 0; mHead.opaque2 = 0;
		mNextPtrOffset = -1;
	}
	void	push_atomic(T *item) {
		if (mNextPtrOffset < 0) {
			T **pnext = &item->next();	// hack around offsetof not working with C++
			mNextPtrOffset = (char *)pnext - (char *)item;
		}
		OSAtomicEnqueue(&mHead, item, mNextPtrOffset);
	}

	T *		pop_atomic() { return (T *)OSAtomicDequeue(&mHead, mNextPtrOffset); }

	// caution: do not try to implement pop_all_reversed here. the writer could add new elements
	// while the reader is trying to pop old ones!
	
private:
	OSQueueHead		mHead;
	ssize_t			mNextPtrOffset;
};

#else   // !MACOSX

template <typename T>
class TAtomicStack2 {
private:
    struct Node {
        T *data;
        Node* next;

        Node(T *val) : data(val), next(nullptr) {}
    };

    std::atomic<Node*> head;

public:
    TAtomicStack2() : head(nullptr) {}

    void push_atomic(T *value) {
        Node* newNode = new Node(value);
        newNode->next = head.load(std::memory_order_relaxed);

        while (!head.compare_exchange_weak(newNode->next, newNode,
                                           std::memory_order_release,
                                           std::memory_order_relaxed))
            ; // spin
    }

    T * pop_atomic() {
        Node* oldHead = head.load(std::memory_order_relaxed);

        while (oldHead && !head.compare_exchange_weak(oldHead, oldHead->next,
                                                      std::memory_order_acquire,
                                                      std::memory_order_relaxed))
            ; // spin

        if (oldHead) {
            T *value = oldHead->data;
            delete oldHead;
            return value;
        }

        return NULL;
    }
};

#endif	// MACOSX

#endif // __CAAtomicStack_h__
