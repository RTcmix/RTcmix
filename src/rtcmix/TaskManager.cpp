//
// C++ Implementation: TaskManager
//
// Description: 
//
//
// Author: Douglas Scott <netdscott@netscape.net>, (C) 2010
//
//


#include "TaskManager.h"
#include "RTSemaphore.h"
#include "RTThread.h"
#include "rt_types.h"
#include <pthread.h>
#include <stdio.h>
#include <assert.h>

#ifdef LINUX
#include <sys/time.h>
#include <sys/resource.h>
#endif

#undef DEBUG
#undef TASK_DEBUG
#undef THREAD_DEBUG
#undef POOL_DEBUG

class Notifiable {
public:
	virtual void notify(int inIndex) = 0;
};

class Notifier {
public:
	Notifier(Notifiable *inTarget, int inIndex) : mTarget(inTarget), mIndex(inIndex) {}
	void notify() { mTarget->notify(mIndex); }
#ifndef THREAD_DEBUG
private:
#endif
	Notifiable	*mTarget;
	int 		mIndex;
};

class TaskThread : public RTThread, Notifier
{
public:
	TaskThread(Notifiable *inTarget, int inIndex)
		: RTThread(inIndex), Notifier(inTarget, inIndex), mTask(NULL) {}
	~TaskThread() { setTask(NULL); start(); }
	inline void setTask(Task *inTask);
	inline void start();
protected:
	virtual void run();
private:
	RTSemaphore	mSema;
	Task *		mTask;
};

inline void TaskThread::setTask(Task *inTask)
{
	mTask = inTask;
}

inline void TaskThread::start()
{
#ifdef THREAD_DEBUG
	//	printf("TaskThread::start(%p): posting for Task %p\n", this, mTask);
#endif
	mSema.post();
}


void TaskThread::run()
{
#ifdef THREAD_DEBUG
	printf("TaskThread %p running\n", this);
#endif
	bool done = false;
	do {
		mSema.wait();
#ifdef THREAD_DEBUG
		printf("TaskThread %d woke up -- about to run mTask %p\n", mIndex, mTask);
#endif
		if (mTask) {
			mTask->run();
#ifdef THREAD_DEBUG
            //printf("\tmTask %p done\n", mTask);
            printf("TaskThread %d task done\n", mIndex);
#endif
			delete mTask;
		}
		else {
			done = true;
		}
		notify();
	}
	while (!done);
#ifdef THREAD_DEBUG
	printf("TaskThread %p exiting\n", this);
#endif
}

class ThreadPool : private Notifiable
{
public:
	ThreadPool() : mThreadSema(RT_THREAD_COUNT), mRequestCount(0), mWaitSema(0) { 
		for(int i=0; i<RT_THREAD_COUNT; ++i) {
			mThreads[i] = new TaskThread(this, i);
		}
	}
	~ThreadPool() {
		for(int i=0; i<RT_THREAD_COUNT; ++i)
			delete mThreads[i];
	}
	inline TaskThread &getAvailableThread();
	virtual void notify(int inIndex);
	inline void wait();
private:
	TaskThread		*mThreads[RT_THREAD_COUNT];
	RTSemaphore		mThreadSema;
	AtomicInt		mRequestCount;		// Number of thread requests made that have not notified
	RTSemaphore		mWaitSema;
};

inline void ThreadPool::wait() {
	int count = mRequestCount;
	for(int i=0; i<count; ++i)
		mThreads[i]->start();
#ifdef POOL_DEBUG
	printf("ThreadPool waiting on %d threads\n", (int)count);
#endif
	mWaitSema.wait();
}

// Block until a thread is free and return it

TaskThread& ThreadPool::getAvailableThread()
{
    int tIndex = mRequestCount;
    assert(tIndex < RT_THREAD_COUNT);
	++mRequestCount;
#ifdef POOL_DEBUG
	printf("ThreadPool returning thread[%d]\n", tIndex);
#endif
	return *mThreads[tIndex];
}

// Let thread pool know that the thread at index inIndex is available

void ThreadPool::notify(int inIndex)
{
#ifdef POOL_DEBUG
	printf("ThreadPool notified for index %d\n", inIndex);
#endif
	int newCount = --mRequestCount;
#ifdef POOL_DEBUG
    printf("ThreadPool mRequestCount: %d\n", newCount);
#endif
	if (newCount == 0) {
#if defined(POOL_DEBUG) || defined(THREAD_DEBUG)
		printf("ThreadPool posting to wait semaphore\n");
#endif
		mWaitSema.post();
	}
}

TaskManagerImpl::TaskManagerImpl() : mThreadPool(new ThreadPool) {}

TaskManagerImpl::~TaskManagerImpl() { delete mThreadPool; }

void TaskManagerImpl::addTask(Task *inTask)
{
	mThreadPool->getAvailableThread().setTask(inTask);
}

void TaskManagerImpl::wait()
{
#ifdef DEBUG
	printf("TaskManagerImpl::wait waiting on ThreadPool...\n");
#endif
	mThreadPool->wait();
#ifdef DEBUG
	printf("TaskManagerImpl::wait done\n");
#endif
}

TaskManager::TaskManager() : mImpl(new TaskManagerImpl)
{
}

TaskManager::~TaskManager()
{
	delete mImpl;
}

void TaskManager::waitForCompletion()
{
	mImpl->wait();
}
