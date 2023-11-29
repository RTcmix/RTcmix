//
// C++ Implementation: TaskManager
//
// Description: 
//
//
// Author: Douglas Scott <netdscott-at-netscape-dot-net>, (C) 2010
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
    virtual ~Notifiable() {}
	virtual void notify(int inIndex) = 0;
};

class Notifier {
public:
	Notifier(Notifiable *inTarget, int inIndex) : mTarget(inTarget), mIndex(inIndex) {}
	void notify() { mTarget->notify(mIndex); }
protected:
    int getIndex() const { return mIndex; }
#ifndef THREAD_DEBUG
private:
#endif
	Notifiable	*mTarget;
	int 		mIndex;
};

class TaskThread : public RTThread, Notifier
{
public:
	TaskThread(Notifiable *inTarget, TaskProvider *inProvider, int inIndex)
		: RTThread(inIndex), Notifier(inTarget, inIndex),
		  mStopping(false), mTaskProvider(inProvider) { start(); }
	~TaskThread() { mStopping = true; wake(); }
	inline void wake();
protected:
	virtual void	run();
	Task *			getATask() { return mTaskProvider->getSingleTask(); }
private:
	bool			mStopping;
	TaskProvider *	mTaskProvider;
	RTSemaphore		mSema;
};

inline void TaskThread::wake()
{
#ifdef THREAD_DEBUG
	printf("TaskThread::wake: thread %d posting for Task\n", getIndex());
#endif
	mSema.post();
}

#undef TASK_TIME_DEBUG
#ifdef TASK_TIME_DEBUG
#include <mach/mach_time.h>
#endif

void TaskThread::run()
{
#ifdef THREAD_DEBUG
    int tIndex = getIndex();
	printf("TaskThread %d running\n", tIndex);
#endif
    char threadName[16];
    snprintf(threadName, 16, "TaskThread %d", getIndex());
    setName(threadName);
	do {
#ifdef THREAD_DEBUG
		printf("TaskThread %d sleeping...\n", tIndex);
#endif
		mSema.wait();
#ifdef THREAD_DEBUG
		printf("TaskThread %d woke up -- running task loop\n", tIndex);
#endif
#ifdef TASK_TIME_DEBUG
        const uint64_t startTime = mach_absolute_time();
        bool taskWasRun = false;
#endif
        Task *task;
        while ((task = getATask()) != NULL) {
#ifdef TASK_TIME_DEBUG
            taskWasRun = true;
#endif
#ifdef THREAD_DEBUG
            printf("TaskThread %d running task %p...\n", tIndex, task);
#endif
			task->run();
#ifdef THREAD_DEBUG
            printf("TaskThread %d task %p done\n", tIndex, task);
#endif
		}
#ifdef TASK_TIME_DEBUG
        if (taskWasRun) {
            const uint64_t endTime = mach_absolute_time();
            // Time elapsed in Mach time units.
            const uint64_t elapsedMTU = endTime - startTime;

            // Get information for converting from MTU to nanoseconds
            mach_timebase_info_data_t info;
            mach_timebase_info(&info);
            // Get elapsed time in nanoseconds:
            const double elapsedNS = (double)elapsedMTU * (double)info.numer / (double)info.denom;
            printf("TaskThread %d loop done in %.5f ms\n", getIndex(), elapsedNS*1.0e-06);
        }
#endif
		notify();
	}
	while (!mStopping);
#ifdef THREAD_DEBUG
	printf("TaskThread %d exiting\n", tIndex);
#endif
}

class ThreadPool : private Notifiable
{
public:
	ThreadPool(TaskProvider *inProvider) : mRequestCount(0), mThreadSema(RT_THREAD_COUNT), mWaitSema(0) {
		for(int i=0; i<RT_THREAD_COUNT; ++i) {
			mThreads[i] = new TaskThread(this, inProvider, i);
		}
	}
	virtual ~ThreadPool() {
		for(int i=0; i<RT_THREAD_COUNT; ++i)
			delete mThreads[i];
	}
	virtual void notify(int inIndex);
	inline void startAndWait(int taskCount);
private:
	TaskThread		*mThreads[RT_THREAD_COUNT];
	AtomicInt		mRequestCount;
	RTSemaphore		mThreadSema;
	RTSemaphore		mWaitSema;
};

inline void ThreadPool::startAndWait(int taskCount) {
	// Dont wake any more threads than we have tasks.
	mRequestCount = (int) std::min(taskCount, RT_THREAD_COUNT);
	const int count = (int) mRequestCount;
	for(int i=0; i<count; ++i)
		mThreads[i]->wake();
#ifdef POOL_DEBUG
	printf("ThreadPool::startAndWait: waiting on %d threads\n", count);
#endif
	mWaitSema.wait();
}

// Let thread pool know that the thread at index inIndex is available

void ThreadPool::notify(int inIndex)
{
#ifdef POOL_DEBUG
	printf("ThreadPool notified for index %d\n", inIndex);
#endif
	if (mRequestCount.decrementAndTest())
	{
#if defined(POOL_DEBUG) || defined(THREAD_DEBUG)
		printf("ThreadPool posting to wait semaphore\n");
#endif
		mWaitSema.post();
	}
}

TaskManagerImpl::TaskManagerImpl()
	: mThreadPool(new ThreadPool(this)), mTaskHead(NULL), mTaskTail(NULL) {}

TaskManagerImpl::~TaskManagerImpl() { delete mThreadPool; }

void TaskManagerImpl::addTask(Task *inTask)
{
#ifdef DEBUG
	printf("TaskManagerImpl::addTask: adding task %p to linked list\n", inTask);
#endif
	// Each new task is put behind the previous one
	inTask->next() = mTaskHead;
	mTaskHead = inTask;
}

Task * TaskManagerImpl::getSingleTask()
{
	Task *task = mTaskStack.pop_atomic();
#ifdef DEBUG
	printf("TaskManagerImpl::getSingleTask: returning task %p from stack\n", task);
#endif
	return task;
}

void TaskManagerImpl::startAndWait()
{
#ifdef DEBUG
	printf("TaskManagerImpl::startAndWait pushing tasks onto stack\n");
#endif
	int taskCount = 0;
	// Push entire reversed linked list into stack
	for (Task *t = mTaskHead; t != NULL; ++taskCount) {
		Task *next = t->next();
		mTaskStack.push_atomic(t);
		t = next;
	}
	mTaskHead = mTaskTail = NULL;
#ifdef DEBUG
    printf("TaskManagerImpl::startAndWait waiting on ThreadPool for %d tasks...\n", taskCount);
#endif
	mThreadPool->startAndWait(taskCount);
#ifdef DEBUG
	printf("TaskManagerImpl::startAndWait done\n");
#endif
}

TaskManager::TaskManager() : mImpl(new TaskManagerImpl)
{
}

TaskManager::~TaskManager()
{
	delete mImpl;
}

