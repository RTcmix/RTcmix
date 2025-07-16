//
//  thread_tester.cpp
//

#if 0

#include <libkern/OSAtomic.h>
#include <sys/types.h>
#include <dispatch/dispatch.h>
#include <assert.h>
#include <stdio.h>

#define THREAD_DEBUG
#define DEBUG
#define TASK_DEBUG
#define POOL_DEBUG

#define MACOSX      /* ONLY RUN THIS TEST CODE ON A MAC! */
#define __LP64__ 1

#ifndef NULL
#define NULL 0
#endif

class AtomicInt
{
    int32_t val;
public:
    AtomicInt(int inVal=0) : val(inVal) {}
    operator int () const { return val; }
    void increment() { OSAtomicIncrement32(&val); }
    bool incrementAndTest() { return OSAtomicIncrement32(&val) == 0; }
    bool decrementAndTest() { return OSAtomicDecrement32(&val) == 0; }
    int operator = (int rhs) { return (val = rhs); }
    
};

//  linked list LIFO or FIFO (pop_all_reversed) stack, elements are pushed and popped atomically
//  class T must implement T *& next().
template <class T>
class TAtomicStack {
public:
    TAtomicStack() : mHead(NULL) { }
    
    // non-atomic routines, for use when initializing/deinitializing, operate NON-atomically
    void    push_NA(T *item)
    {
        item->next() = mHead;
        mHead = item;
    }
    
    T *        pop_NA()
    {
        T *result = mHead;
        if (result)
            mHead = result->next();
        return result;
    }
    
    bool    empty() { return mHead == NULL; }
    
    T *        head() { return mHead; }
    
    // atomic routines
    void    push_atomic(T *item)
    {
        T *head_;
        do {
            head_ = mHead;
            item->next() = head_;
        } while (!compare_and_swap(head_, item, &mHead));
    }
    
    void    push_multiple_atomic(T *item)
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
    
    T *        pop_atomic_single_reader()
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
    
    T *        pop_atomic()
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
    
    T *        pop_all()
    {
        T *result;
        do {
            if ((result = mHead) == NULL)
                break;
        } while (!compare_and_swap(result, NULL, &mHead));
        return result;
    }
    
    T*        pop_all_reversed()
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
    
    static bool    compare_and_swap(T *oldvalue, T *newvalue, T **pvalue)
    {
#ifdef MACOSX
#if __LP64__
        return ::OSAtomicCompareAndSwap64Barrier(int64_t(oldvalue), int64_t(newvalue), (int64_t *)pvalue);
#elif MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
        return ::OSAtomicCompareAndSwap32Barrier(int32_t(oldvalue), int32_t(newvalue), (int32_t *)pvalue);
#else
        return ::CompareAndSwap(UInt32(oldvalue), UInt32(newvalue), (UInt32 *)pvalue);
#endif
#else    // MACOSX
#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100
        return __sync_bool_compare_and_swap(pvalue, oldvalue, newvalue);
#else
        #error WE NEED AN ATOMIC COMPARE AND SWAP OPERATOR HERE
#endif
#endif    // MACOSX
    }
    
protected:
    T *        mHead;
};

class CAAtomicStack {
public:
    CAAtomicStack(size_t nextPtrOffset) : mNextPtrOffset(nextPtrOffset) {
        /*OSQueueHead h = OS_ATOMIC_QUEUE_INIT; mHead = h;*/
        mHead.opaque1 = 0; mHead.opaque2 = 0;
    }
    // a subset of the above
    void    push_atomic(void *p) { OSAtomicEnqueue(&mHead, p, mNextPtrOffset); }
    void    push_NA(void *p) { push_atomic(p); }
    
    void *    pop_atomic() { return OSAtomicDequeue(&mHead, mNextPtrOffset); }
    void *    pop_atomic_single_reader() { return pop_atomic(); }
    void *    pop_NA() { return pop_atomic(); }
    
private:
    OSQueueHead        mHead;
    size_t            mNextPtrOffset;
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
    void    push_atomic(T *item) {
        if (mNextPtrOffset < 0) {
            T **pnext = &item->next();    // hack around offsetof not working with C++
            mNextPtrOffset = (char *)pnext - (char *)item;
        }
        OSAtomicEnqueue(&mHead, item, mNextPtrOffset);
    }
    void    push_NA(T *item) { push_atomic(item); }
    void    push_all_NA(T *item) { }
    
    T *        pop_atomic() { return (T *)OSAtomicDequeue(&mHead, mNextPtrOffset); }
    T *        pop_atomic_single_reader() { return pop_atomic(); }
    T *        pop_NA() { return pop_atomic(); }
    
    // caution: do not try to implement pop_all_reversed here. the writer could add new elements
    // while the reader is trying to pop old ones!
    
private:
    OSQueueHead        mHead;
    ssize_t            mNextPtrOffset;
};

#include <pthread.h>

class RTThread
{
public:
    RTThread(int inThreadIndex);
    virtual ~RTThread();
    static int    GetIndexForThread();
protected:
    void start();
    virtual void run()=0;
    static void *sProcess(void *inContext);
private:
    int            GetIndex() const { return mThreadIndex; }
    static void    InitOnce();
    static void    SetIndexForThread(int inIndex);
    static void DestroyMemory(void *value);
    
    pthread_t                mThread;
    int                        mThreadIndex;
    static pthread_key_t    sIndexKey;
};

static pthread_once_t sOnceControl = PTHREAD_ONCE_INIT;

pthread_key_t    RTThread::sIndexKey;

RTThread::RTThread(int inThreadIndex)
    : mThread(NULL), mThreadIndex(inThreadIndex) {
    pthread_once(&sOnceControl, InitOnce);
}

RTThread::~RTThread() {
    if (mThread) {
        pthread_join(mThread, NULL);
    }
}

// We cannot start running the pthread in the ctor, so we do it here.

void RTThread::start() {
    if (mThread == NULL) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_RR);
        if (pthread_create(&mThread, &attr, sProcess, this) != 0) {
            throw -1;
        }
        pthread_attr_destroy(&attr);
    }
}

// static internals

void RTThread::InitOnce() {
    int status = pthread_key_create(&sIndexKey, RTThread::DestroyMemory);
    assert(status == 0);
}

int RTThread::GetIndexForThread() {
    void *mem = pthread_getspecific(sIndexKey);
    assert(mem != NULL);
    return *((int *) mem);
}

void RTThread::SetIndexForThread(int inIndex) {
    int *pIndex = new int;
    *pIndex = inIndex;
    int status = pthread_setspecific(sIndexKey, pIndex);
    assert(status == 0);
}

void RTThread::DestroyMemory(void *value)
{
    int *threadMem = (int *) value;
    delete threadMem;
    pthread_setspecific(sIndexKey, NULL);
}

void *RTThread::sProcess(void *inContext)
{
    RTThread *This = (RTThread *) inContext;
    SetIndexForThread(This->GetIndex());
    if (setpriority(PRIO_PROCESS, 0, -20) != 0) {
#ifdef THREAD_DEBUG
        perror("RTThread::sProcess: setpriority() failed.");
#endif
    }
    This->run();
    return NULL;
}



#include <vector>

#ifndef RT_THREAD_COUNT
#define RT_THREAD_COUNT 8
#endif

using namespace std;

class Task
{
public:
    Task() : mNext(NULL) {}
    virtual ~Task() {}
    virtual void run()=0;
    Task *&    next() { return mNext; }
private:
    Task    *mNext;
};

class TaskProvider {
public:
    virtual ~TaskProvider() {}
    virtual Task *    getSingleTask() = 0;
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
    Ret        mReturned;
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
    Arg        mArg;
    Ret        mReturned;
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
    Arg1    mArg1;
    Arg2    mArg2;
    Ret        mReturned;
};

class ThreadPool;

class TaskManagerImpl : public TaskProvider
{
public:
    TaskManagerImpl();
    virtual ~TaskManagerImpl();
    virtual Task *    getSingleTask();
    void    addTask(Task *inTask);
    void    startAndWait();
private:
    ThreadPool *            mThreadPool;
    Task *                    mTaskHead;
    Task *                    mTaskTail;
    TAtomicStack2<Task>        mTaskStack;
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
    TaskManagerImpl    *mImpl;
};

template <typename Object, typename Ret, Ret (Object::*Method)()>
inline void TaskManager::addTask(Object * inObject)
{
    mImpl->addTask(new NoArgumentTask<Object, Ret, Method>(inObject));
}

template <typename Object, typename Ret, typename Arg, Ret (Object::*Method)(Arg)>
inline void TaskManager::addTask(Object * inObject, Arg inArg)
{
    mImpl->addTask(OneArgumentTask<Object, Ret, Arg, Method>(inObject, inArg));
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

class RTSemaphore
{
public:
    RTSemaphore(unsigned inStartingValue=0) : mSema(dispatch_semaphore_create((long)inStartingValue)) { if (!mSema) { throw -1; }; }
    ~RTSemaphore() { dispatch_release(mSema); mSema = NULL; }
    void wait() { dispatch_semaphore_wait(mSema, DISPATCH_TIME_FOREVER); }    // each thread will wait on this
    void post() { dispatch_semaphore_signal(mSema); }    // when done, each thread calls this
private:
    dispatch_semaphore_t    mSema;
};

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
	//	printf("TaskThread::wake(%p): posting for Task %p\n", this, mTask);
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
#endif
#ifdef EXPERIMENT
        Task *task = getAllTasks();
        while (task != NULL) {
#else
        Task *task = getATask();
        if (task != NULL) {
#endif
#ifdef THREAD_DEBUG
            printf("TaskThread %d running task %p...\n", tIndex, task);
#endif
			task->run();
#ifdef THREAD_DEBUG
            printf("TaskThread %d task %p done\n", tIndex, task);
#endif
#ifdef EXPERIMENT
            Task *next = task->next();
            delete task;
            task = next;
#endif
		}
#ifdef TASK_TIME_DEBUG
        const uint64_t endTime = mach_absolute_time();
        // Time elapsed in Mach time units.
        const uint64_t elapsedMTU = endTime - startTime;

        // Get information for converting from MTU to nanoseconds
        mach_timebase_info_data_t info;
        mach_timebase_info(&info);
        // Get elapsed time in nanoseconds:
        const double elapsedNS = (double)elapsedMTU * (double)info.numer / (double)info.denom;
        printf("TaskThread %d loop done in %.5f ms\n", getIndex(), elapsedNS*1.0e-06);
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
	mRequestCount = std::min(taskCount, RT_THREAD_COUNT);
	const int count = mRequestCount;
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
    OSMemoryBarrier();
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

#include <math.h>

class TestObject
{
public:
    TestObject() {}
    int run();
};

int TestObject::run() {
    int idx = RTThread::GetIndexForThread();
//    printf("TestObject running with thread index %d\n", idx);
    for (int n = 0; n < 100000; ++n) {
        double value = (n + 3) * 1.54366;
        double x = log(value) * sqrt(value) * sin(value) * value;
        x = x - (double) n;
    }
    return 1;
}


//                 taskManager->addTask<Instrument, int, BusType, int, &Instrument::exec>(Iptr, bus_type, bus);

int main(int argc, char **argv)
{
    TestObject *testArray = new TestObject[64];
    
    TaskManager *testMgr = new TaskManager;
    vector<TestObject *>tests;

    while (true) {
        for (int n = 0; n < 64; ++n) {
            TestObject *theTest = &testArray[n];
            tests.push_back(theTest);
            testMgr->addTask<TestObject, int, &TestObject::run>(theTest);
        }
        testMgr->waitForTasks(tests);
        tests.clear();
        usleep(1000);
    }
    return 0;
}
#endif
