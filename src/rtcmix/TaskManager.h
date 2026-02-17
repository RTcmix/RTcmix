//
// C++ Interface: TaskManager
//
// Description:
//
//
// Author: Douglas Scott <netdscott-at-netscape-dot-net>, (C) 2010
//
// Nested parallelism support added 2024 for pull-based audio routing
//

#ifndef _TASKMANAGER_H_
#define _TASKMANAGER_H_

#include <vector>
#include "atomic_stack.h"
#include "Lockable.h"
#include "rt_types.h"

#ifndef RT_THREAD_COUNT
#define RT_THREAD_COUNT 2
#endif

using namespace std;

class WaitContext;

class Task
{
public:
	Task() : mNext(NULL), mContext(NULL) {}
	virtual ~Task() {}
	virtual void run()=0;
	Task *&	next() { return mNext; }
	void setContext(WaitContext *ctx) { mContext = ctx; }
	WaitContext *getContext() const { return mContext; }
private:
	Task	*mNext;
	WaitContext *mContext;
};

class TaskProvider {
public:
    virtual ~TaskProvider() {}
	virtual Task *	getSingleTask() = 0;
};

template <typename Object, typename Ret, Ret (Object::*Method)()>
class NoArgumentTask : public Task
{
public:
	NoArgumentTask(Object *inObject) : mObject(inObject), mReturned(0) {}
	NoArgumentTask(const NoArgumentTask &rhs) : mObject(rhs.mObject), mReturned(0) {}
	virtual ~NoArgumentTask() {}
	virtual void run() { mReturned = (mObject->*Method)(); }
private:
	Object *mObject;
	Ret		mReturned;
};

template <typename Object, typename Ret, typename Arg, Ret (Object::*Method)(Arg)>
class OneArgumentTask : public Task
{
public:
	OneArgumentTask(Object *inObject, Arg inArg)
: mObject(inObject), mArg(inArg), mReturned(0) {}
	OneArgumentTask(const OneArgumentTask &rhs) : mObject(rhs.mObject), mArg(rhs.mArg), mReturned(0) {}
	virtual ~OneArgumentTask() {}
	virtual void run() { mReturned = (mObject->*Method)(mArg); }
private:
	Object *mObject;
	Arg		mArg;
	Ret		mReturned;
};

template <typename Object, typename Ret, typename Arg1, typename Arg2, Ret (Object::*Method)(Arg1, Arg2)>
class TwoArgumentTask : public Task
{
public:
	TwoArgumentTask(Object *inObject, Arg1 inArg1, Arg2 inArg2)
: mObject(inObject), mArg1(inArg1), mArg2(inArg2), mReturned(0) {}
	TwoArgumentTask(const TwoArgumentTask &rhs)
: mObject(rhs.mObject), mArg1(rhs.mArg1), mArg2(rhs.mArg2), mReturned(0) {}
	virtual ~TwoArgumentTask() {}
	virtual void run() { mReturned = (mObject->*Method)(mArg1, mArg2); }
private:
	Object *mObject;
	Arg1	mArg1;
	Arg2	mArg2;
	Ret		mReturned;
};

class ThreadPool;
class RTSemaphore;

/**
 * WaitContext - Per-invocation synchronization for nested parallelism.
 *
 * Each call to startAndWait() creates a WaitContext that tracks completion
 * of that specific batch of tasks. This allows nested calls (e.g., from within
 * a running task) without conflicting with outer invocations.
 */
class WaitContext
{
public:
	WaitContext(int taskCount);
	~WaitContext();

	// Called when a task from this context completes
	void taskCompleted();

	// Check if all tasks are done
	bool isComplete() const;

	// Wait for completion (called only if stack exhausted but tasks remain)
	void wait();

private:
	AtomicInt mRemainingTasks;
	RTSemaphore *mSema;
};

class TaskManagerImpl : public TaskProvider, public Lockable
{
public:
	TaskManagerImpl();
	virtual ~TaskManagerImpl();
	virtual Task *	getSingleTask();
	void	addTask(Task *inTask);
	void	startAndWait();
private:
	ThreadPool *			mThreadPool;
	Task *					mTaskHead;
	Task *					mTaskTail;
	TAtomicStack2<Task>		mTaskStack;
};

class TaskManager
{
public:
	TaskManager();
	~TaskManager();
	template <typename Object, typename Ret, Ret (Object::*Method)()>
	inline void addTask(Object * inObject);
	template <typename Object, typename Ret, typename Arg, Ret (Object::*Method)(Arg)>
	inline void addTask(Object * inObject, Arg inArg);
	template <typename Object, typename Ret, typename Arg1, typename Arg2, Ret (Object::*Method)(Arg1, Arg2)>
	inline void addTask(Object * inObject, Arg1 inArg1, Arg2 inArg2);
	template <typename Object>
	inline void waitForTasks(vector<Object *> &ioVector);
private:
	TaskManagerImpl	*mImpl;
};

template <typename Object, typename Ret, Ret (Object::*Method)()>
inline void TaskManager::addTask(Object * inObject)
{
	mImpl->addTask(new NoArgumentTask<Object, Ret, Method>(inObject));
}

template <typename Object, typename Ret, typename Arg, Ret (Object::*Method)(Arg)>
inline void TaskManager::addTask(Object * inObject, Arg inArg)
{
	mImpl->addTask(new OneArgumentTask<Object, Ret, Arg, Method>(inObject, inArg));
}

template <typename Object, typename Ret, typename Arg1, typename Arg2, Ret (Object::*Method)(Arg1, Arg2)>
inline void TaskManager::addTask(Object * inObject, Arg1 inArg1, Arg2 inArg2)
{
	mImpl->addTask(new TwoArgumentTask<Object, Ret, Arg1, Arg2, Method>(inObject, inArg1, inArg2));
}

template <typename Object>
inline void TaskManager::waitForTasks(vector<Object *> &ioVector)
{
	mImpl->startAndWait();
}

#endif	// _TASKMANAGER_H_
