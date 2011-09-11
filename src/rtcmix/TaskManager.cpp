//
// C++ Implementation: TaskManager
//
// Description: 
//
//
// Author: Douglas Scott <netdscott@netscape.net>, (C) 2010
//
//

#ifndef RT_THREAD_COUNT
#define RT_THREAD_COUNT 2
#endif


#include "TaskManager.h"
#include "Lockable.h"
#include <pthread.h>
#include <stack>
#include <stdio.h>
#include <assert.h>

#undef DEBUG

#ifdef MACOSX

#include <dispatch/dispatch.h>

class Semaphore
{
public:
	Semaphore(unsigned inStartingValue=0) : mSema(dispatch_semaphore_create((long)inStartingValue)) {}
	~Semaphore() { mSema = NULL; }
	void wait() { dispatch_semaphore_wait(mSema, DISPATCH_TIME_FOREVER); }	// each thread will wait on this
	void post() { dispatch_semaphore_signal(mSema); }	// when done, each thread calls this
private:
	dispatch_semaphore_t	mSema;
};

#else

#include <semaphore.h>

class Semaphore
{
public:
	Semaphore(unsigned inStartingValue=0) { sem_init(&mSema, 0, inStartingValue); }
	~Semaphore() { sem_destroy(&mSema); }
	void wait() { sem_wait(&mSema); }	// each thread will wait on this
	void post() { sem_post(&mSema); }	// when done, each thread calls this
private:
	sem_t	mSema;
};

#endif

class Notifiable {
public:
	virtual void notify(int inIndex) = 0;
};

class Notifier {
public:
	Notifier(Notifiable *inTarget, int inIndex) : mTarget(inTarget), mIndex(inIndex) {}
	void notify() { mTarget->notify(mIndex); }
private:
	Notifiable	*mTarget;
	int 		mIndex;
};

class TaskThread : public Notifier
{
public:
	TaskThread(Notifiable *inTarget, int inIndex)
		: Notifier(inTarget, inIndex), mTask(NULL)
	{
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_RR);
		pthread_create(&mThread, &attr, sProcess, this);
        pthread_attr_destroy(&attr);
	}
	~TaskThread() { start(NULL); pthread_join(mThread, NULL); }
	inline void start(Task *inTask);
protected:
	void run();
	static void *sProcess(void *inContext);
private:
	pthread_t	mThread;
	Semaphore	mSema;
	Task *		mTask;
};

inline void TaskThread::start(Task *inTask)
{
	mTask = inTask;
#ifdef DEBUG
	printf("TaskThread::start(%p): posting for Task %p\n", this, inTask);
#endif
	mSema.post();
}

void TaskThread::run()
{
#ifdef DEBUG
	printf("TaskThread %p running\n", this);
#endif
	bool done = false;
	do {
		mSema.wait();
#ifdef DEBUG
		printf("TaskThread %p woke up -- about to run mTask %p\n", this, mTask);
#endif
		if (mTask) {
			mTask->run();
			delete mTask;
		}
		else {
			done = true;
		}
		notify();
	}
	while (!done);
#ifdef DEBUG
	printf("TaskThread %p exiting\n", this);
#endif
}

void *TaskThread::sProcess(void *inContext)
{
	TaskThread *This = (TaskThread *) inContext;
    if (setpriority(PRIO_PROCESS, 0, -20) != 0) {
        perror("TaskThread::sProcess: setpriority() failed.");
    }
	This->run();
	return NULL;
}

class ThreadPool : private Lockable, private Notifiable
{
public:
	ThreadPool() : mThreadSema(RT_THREAD_COUNT), mRequestCount(0), mWaitSema(0) { 
		for(int i=0; i<RT_THREAD_COUNT; ++i) {
			mThreads[i] = new TaskThread(this, i);
			mIndices.push(i);
		}
	}
	~ThreadPool() {
		for(int i=0; i<RT_THREAD_COUNT; ++i)
			delete mThreads[i];
	}
	inline TaskThread &getAvailableThread();
	virtual void notify(int inIndex);
	void setRequestCount(int count) { mRequestCount = count; }
	void wait() { mWaitSema.wait(); }
private:
	TaskThread		*mThreads[RT_THREAD_COUNT];
	std::stack<int>	mIndices;
	Semaphore		mThreadSema;
	int				mRequestCount;		// Number of thread requests made that have not notified
	Semaphore		mWaitSema;
};

// Block until a thread is free and return it

TaskThread& ThreadPool::getAvailableThread()
{
	mThreadSema.wait();
	lock();
	int tIndex = mIndices.top();
	mIndices.pop();
	unlock();
#ifdef DEBUG
	printf("ThreadPool returning thread[%d]\n", tIndex);
#endif
	return *mThreads[tIndex];
}

// Let thread pool know that the thread at index inIndex is available

void ThreadPool::notify(int inIndex)
{
	lock();
#ifdef DEBUG
	printf("ThreadPool notified for index %d\n", inIndex);
#endif
	mIndices.push(inIndex);
	int newCount = --mRequestCount;
	unlock();
	mThreadSema.post();
	if (newCount == 0) {
#ifdef DEBUG
		printf("ThreadPool mRequestCount: %d\n", newCount);
		printf("ThreadPool posting to wait semaphore\n");
#endif
		mWaitSema.post();
	}
}

TaskManagerImpl::TaskManagerImpl() : mThreadPool(new ThreadPool) {}

TaskManagerImpl::~TaskManagerImpl() { delete mThreadPool; }

void TaskManagerImpl::addTask(Task *inTask)
{
	mThreadPool->getAvailableThread().start(inTask);
}

void TaskManagerImpl::setRequestCount(int count)
{
	mThreadPool->setRequestCount(count);
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
