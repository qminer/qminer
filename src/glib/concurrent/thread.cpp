/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <base.h>
#include "thread.h"

#if defined(GLib_WIN)

#include "win/thread_win.cpp"

#elif defined(GLib_UNIX)

#include "posix/thread_posix.cpp"

#else

// no support for threads in non-Win and non-Unix systems

#endif

TInterruptibleThread& TInterruptibleThread::operator=(const TInterruptibleThread& Other) {
    TThread::operator =(Other);
    SleeperBlocker = Other.SleeperBlocker;
    return *this;
}
    
void TInterruptibleThread::WaitForInterrupt(const int Msecs) {
    SleeperBlocker.Block(Msecs);
}

void TInterruptibleThread::Interrupt() {
    SleeperBlocker.Release();
}


////////////////////////////////////////////
// Thread pool
TThreadPool::TWorkerThread::TWorkerThread():
        TThread(),
        Pool(NULL) {}

TThreadPool::TWorkerThread::TWorkerThread(TWPt<TThreadPool> _Pool):
        TThread(),
        Pool(_Pool) {}

void TThreadPool::TWorkerThread::Run() {
    uint64 ThreadId = GetThreadId();
    while (true) {
        try {
            // get a task, empty task will stop this thread
            printf("Waiting for work, %ld\n", ThreadId);
            TWPt<TRunnable> Runnable = Pool->WaitForTask(ThreadId);
            // check if the executor was terminated
            if (Runnable.Empty()) { 
                printf("Thread got empty runnable, %ld\n", ThreadId);
                break; }
            printf("Thread calling run, %ld\n", ThreadId);
            // run the task
            Runnable->Run();
        } catch (const PExcept& Except) {
            printf("Exception in worker thread %ld: %s\n", ThreadId, Except->GetMsgStr().CStr());
        }
    }

    printf("Thread %ld exiting finished ...\n", ThreadId);
}

TThreadPool::TThreadPool(const int& PoolSize):
        ThreadV(),
        TaskQ(),
        Lock() {
    EAssertR(PoolSize > 0, "Pool size should be greater than zero.");
    printf("Creating pool with %d threads ...\n", PoolSize);

    for (int ThreadN = 0; ThreadN < PoolSize; ThreadN++) {
        ThreadV.Add(TWorkerThread(this));
    }
    printf("Starting %d threads ...\n", PoolSize);
    ThreadV.StartAll();
}

TThreadPool::~TThreadPool() {
    int PoolSize = ThreadV.Len();
    TSysProc::Sleep(3000);
    printf("Destroying pool with %d threads ...\n", PoolSize);
    // Add empty tasks that will stop threads
    for (int ThreadN = 0; ThreadN < PoolSize; ThreadN++) {
        Execute(NULL);
    }

    // join
    printf("Calling join for %d...\n", PoolSize);
    ThreadV.Join();
    printf("Destroyed %d...\n", PoolSize);
}

void TThreadPool::Execute(const TWPt<TRunnable>& Runnable) {
    printf("Acquiring a lock...\n");
    Lock.Lock();

    printf("Adding new runnable to queue ...\n");
    TaskQ.Push(Runnable);
    printf("Sending signal in main thread!!\n");
    Lock.Signal();

    uint64 QSize = TaskQ.Len();

    Lock.Release();

    if (QSize > 0 && QSize % 100 == 0) {
        printf("Task queue has %ld pending tasks!\n", QSize);
    }
}

TWPt<TThreadPool::TRunnable> TThreadPool::WaitForTask(const uint64& ThreadId) {
    Lock.Lock();
    while (TaskQ.Empty()) {
        Lock.WaitForSignal();
        printf("Got signal in thread %ld!!\n", ThreadId);
    }
    printf("Got runnable for thread %ld!!\n", ThreadId);
    TWPt<TRunnable> Runnable = TaskQ.Pop();

    Lock.Release();

    return Runnable;
}
