/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef THREAD_H
#define THREAD_H

#include <base.h>

enum TMutexType {
    mtFast,
    mtRecursive
};

#if defined(GLib_WIN)

#include "win/thread_win.h"

#elif defined(GLib_UNIX)

#include "posix/thread_posix.h"

#else

// no support for threads in non-Win and non-Unix systems

#endif

ClassTPE(TInterruptibleThread, PInterruptipleThread, TThread)// {
protected:
    // Use for interrupting and waiting
    TBlocker SleeperBlocker;
public:
    TInterruptibleThread(): TThread() { }
    TInterruptibleThread(const TInterruptibleThread& Other): TThread() { operator=(Other); }
    TInterruptibleThread& operator=(const TInterruptibleThread& Other);
    
    void WaitForInterrupt(const int Msecs = INFINITE);
    void Interrupt();
};

////////////////////////////////////////////
// Lock 
//   Wrapper around critical section, which automatically enters
//   on construct, and leaves on scope unwinding (destruct)
class TLock {
    friend class TCondVarLock;
private:
    TCriticalSection& CriticalSection;
public:
    TLock(TCriticalSection& _CriticalSection):
        CriticalSection(_CriticalSection) { CriticalSection.Enter(); }
    ~TLock() { CriticalSection.Leave(); }
};

////////////////////////////////////////////
/// Thread pool
///   Contains a pool of worker threads which can execute TRunnable objects. The runnable
///   objects (tasks) form a queue internally. If the queue is empty all worker threads wait
///   for a signal from the pool.
///  Execute function represents the main functionality.
///  IMPORTANT: thread pool takes ownership of runnable objects and will free them after a task
///  executes them.
class TThreadPool {
public:
    /// Has to implement Run()
    class TRunnable {
    public:
        /// Empty default constructor
        TRunnable() {}
        /// Destructor
        virtual ~TRunnable() {}
        /// Main logic to be implemented
        virtual void Run() = 0;
    };
private:
    class TWorkerThread: public TThread {
    private:
        /// Pool is queried for new tasks that are to be run on this thread
        TWPt<TThreadPool> Pool;
        /// Notify
        TWPt<TNotify> Notify;
    public:
        /// Only needed for Vector constructor
        TWorkerThread();
        /// Main constructor
        TWorkerThread(TWPt<TThreadPool> Pool, const TWPt<TNotify>& Notify);
        /// Waits for a task from the pool and executes it.
        void Run();
    };

private:
    /// Vector of threads
    TVec<TWorkerThread> ThreadV;
    /// Queue of tasks (runnable objects)
    TLinkedQueue<TWPt<TRunnable>> TaskQ;
    /// Used for signaling to worker threads when the queue is non-empty (work is to be done)
    TCondVarLock Lock;
    /// Notifiy
    PNotify Notify;

public:
    /// Creates and starts threads. When the pool is generated the threads
    /// wait for the task queue to become non-empty.
    TThreadPool(const int& PoolSize=1, const PNotify& Notify=TNullNotify::New());
    /// Destructor waits for all work to stop
    ~TThreadPool();
    /// Pushes a runnable task to the queue, which will be executed when a thread is free.
    /// IMPORTANT: takes ownership of Runnable and frees memory when the
    /// task is finished.
    void Execute(const TWPt<TRunnable>& Runnable) { Execute(Runnable, true); }
private:
    /// Private version of Execute where NULL checks on Runnable
    /// can be enabled/disabled (set to true in public version).
    /// If NULL checks are disabled and NULL is passed, then the thread
    /// is stopped (internal mechanism used in destructor).
    void Execute(const TWPt<TRunnable>& Runnable, const bool& CheckNullP);
    /// Used by each thread. Threads wait until a runnable object is assigned
    /// to them. Threads pass their id (for tracing).
    TWPt<TRunnable> WaitForTask(const uint64& ThreadId);
};

#endif
