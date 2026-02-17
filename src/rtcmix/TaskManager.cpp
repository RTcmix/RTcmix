//
// C++ Implementation: TaskManager
//
// Description:
//
//
// Author: Douglas Scott <netdscott-at-netscape-dot-net>, (C) 2010
//
// Nested parallelism support added 2024 for pull-based audio routing
//

#include "TaskManager.h"
#include "RTSemaphore.h"
#include "RTThread.h"
#include "rt_types.h"
#include <algorithm>
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
#undef CONTEXT_DEBUG

/* --------------------------------------------------------- WaitContext --- */

WaitContext::WaitContext(int taskCount)
	: mRemainingTasks(taskCount), mSema(new RTSemaphore(0))
{
#ifdef CONTEXT_DEBUG
	printf("WaitContext::WaitContext(%d tasks)\n", taskCount);
#endif
}

WaitContext::~WaitContext()
{
#ifdef CONTEXT_DEBUG
	printf("WaitContext::~WaitContext()\n");
#endif
	delete mSema;
}

void WaitContext::taskCompleted()
{
#ifdef CONTEXT_DEBUG
	printf("WaitContext::taskCompleted() remaining before: %d\n", (int)mRemainingTasks);
#endif
	if (mRemainingTasks.decrementAndTest()) {
#ifdef CONTEXT_DEBUG
		printf("WaitContext::taskCompleted() - all done, posting semaphore\n");
#endif
		mSema->post();
	}
}

bool WaitContext::isComplete() const
{
	return mRemainingTasks == 0;
}

void WaitContext::wait()
{
#ifdef CONTEXT_DEBUG
	printf("WaitContext::wait()\n");
#endif
	mSema->wait();
}

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
			// Notify the task's WaitContext that this task is complete
			WaitContext *ctx = task->getContext();
			assert(ctx != NULL);
			ctx->taskCompleted();
			delete task;
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
		// Note: No longer notify pool - WaitContext handles completion signaling
	}
	while (!mStopping);
#ifdef THREAD_DEBUG
	printf("TaskThread %d exiting\n", tIndex);
#endif
}

class ThreadPool : private Notifiable
{
public:
	ThreadPool(TaskProvider *inProvider) : mProvider(inProvider) {
		for (int i = 0; i < RT_THREAD_COUNT; ++i) {
			mThreads[i] = new TaskThread(this, inProvider, i);
		}
	}
	virtual ~ThreadPool() {
		for (int i = 0; i < RT_THREAD_COUNT; ++i)
			delete mThreads[i];
	}
	virtual void notify(int inIndex);
	inline void wakeThreads(int count);
private:
	TaskThread		*mThreads[RT_THREAD_COUNT];
	TaskProvider	*mProvider;
};

inline void ThreadPool::wakeThreads(int count) {
	// Wake up to 'count' threads to help process tasks
	const int threadsToWake = std::min(count, RT_THREAD_COUNT);
#ifdef POOL_DEBUG
	printf("ThreadPool::wakeThreads: waking %d threads\n", threadsToWake);
#endif
	for (int i = 0; i < threadsToWake; ++i) {
		mThreads[i]->wake();
	}
}

// Called by TaskThread when it has no more tasks - currently a no-op
// since WaitContext handles completion signaling
void ThreadPool::notify(int inIndex)
{
#ifdef POOL_DEBUG
	printf("ThreadPool::notify(%d) - no-op, WaitContext handles completion\n", inIndex);
#endif
	(void)inIndex;  // unused
}

TaskManagerImpl::TaskManagerImpl()
	: mThreadPool(new ThreadPool(this)), mTaskHead(NULL) {}

TaskManagerImpl::~TaskManagerImpl() { delete mThreadPool; }

void TaskManagerImpl::addTask(Task *inTask)
{
	AutoLock al(this);
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
	/*
	 * Submission phase: protected by Lockable mutex.
	 *
	 * Multiple InstrumentBus instances may call addTask + startAndWait
	 * concurrently from different TaskThreads (sibling buses pulling
	 * in parallel).  The mTaskHead linked list is not atomic, so we
	 * serialize the submission phase — count, set context, push to the
	 * atomic stack, clear head, wake threads.
	 *
	 * The wait phase runs mutex-free: once tasks are on the atomic stack
	 * with their WaitContext set, everything is lock-free.
	 */

	int taskCount;
	WaitContext *context = NULL;

	{
		AutoLock submitLock(this);

#ifdef DEBUG
		printf("TaskManagerImpl::startAndWait pushing tasks onto stack\n");
#endif
		// Count tasks
		taskCount = 0;
		for (Task *t = mTaskHead; t != NULL; t = t->next()) {
			++taskCount;
		}

		if (taskCount == 0) {
			return;
		}

		// Create WaitContext for this batch (heap-allocated so it
		// survives beyond the locked scope)
		context = new WaitContext(taskCount);

		// Set context on all tasks and push to stack
		for (Task *t = mTaskHead; t != NULL; ) {
			t->setContext(context);
			Task *next = t->next();
			mTaskStack.push_atomic(t);
			t = next;
		}
		mTaskHead = NULL;

#ifdef DEBUG
		printf("TaskManagerImpl::startAndWait waking threads for %d tasks...\n", taskCount);
#endif
		// Wake worker threads to help
		mThreadPool->wakeThreads(taskCount);

	}	// AutoLock releases here — wait phase is mutex-free

	// Check if this thread can participate (is a TaskThread with valid index)
	// TaskThreads must participate to avoid deadlock in nested calls.
	// Main thread cannot participate (no thread-local storage set up).
	if (RTThread::IsTaskThread()) {
		// Participatory wait: help run tasks until all are complete
		while (!context->isComplete()) {
			Task *task = mTaskStack.pop_atomic();
			if (task != NULL) {
				task->run();
				WaitContext *ctx = task->getContext();
				assert(ctx != NULL);
				ctx->taskCompleted();
				delete task;
			} else {
				// Stack is empty but tasks still running on other threads
				// Wait for context completion
				context->wait();
				break;
			}
		}
	} else {
		// Non-TaskThread (e.g., main thread): just wait for completion
		context->wait();
	}

	delete context;

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

