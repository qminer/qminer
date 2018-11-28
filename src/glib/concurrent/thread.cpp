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
        Pool(NULL),
        Notify(NULL) {}

TThreadPool::TWorkerThread::TWorkerThread(TWPt<TThreadPool> Pool, const TWPt<TNotify>& Notify):
        TThread(),
        Pool(Pool),
        Notify(Notify) {}

void TThreadPool::TWorkerThread::Run() {
    uint64 ThreadId = GetThreadId();
    while (true) {
        try {
            // get a task, empty task will stop this thread
            TWPt<TRunnable> Runnable = Pool->WaitForTask(ThreadId);
            // thread is stopped by thread pool by sending a NULL pointer
            if (Runnable.Empty()) {
                break;
            }
            Notify->OnNotifyFmt(TNotifyType::ntInfo, "Calling run in thread [%ld]", ThreadId);
            // run the task
            Runnable->Run();
            Pool->OnRunnableDone();
            // free memory (runnable not empty)
            Runnable.Del();
        } catch (const PExcept& Except) {
            Notify->OnNotifyFmt(TNotifyType::ntErr, "Unhandeled exception in worker thread [%ld]: %s", ThreadId, Except->GetMsgStr().CStr());
        } catch (...) {
            Notify->OnNotifyFmt(TNotifyType::ntErr, "Unknown unhandeled exception in worker thread [%ld]", ThreadId);
        }
    }

    Notify->OnNotifyFmt(TNotifyType::ntInfo, "Exiting thread [%ld]", ThreadId);
}

void TThreadPool::OnRunnableDone() {
    Lock.Lock();
    OutstandingTasks--;
    printf("%d outstaning\n", OutstandingTasks.Val);
    Lock.Release();
}

TThreadPool::TThreadPool(const int& PoolSize, const PNotify& Notify):
        ThreadV(),
        TaskQ(),
        Lock(),
        Notify(Notify) {
    EAssertR(PoolSize > 0, "Pool size should be greater than zero.");
    Notify->OnNotifyFmt(TNotifyType::ntInfo, "Creating a pool with %d threads ...", PoolSize);
    for (int ThreadN = 0; ThreadN < PoolSize; ThreadN++) {
        ThreadV.Add(TWorkerThread(this, Notify()));
        ThreadV[ThreadN].Start();
    }
}

TThreadPool::~TThreadPool() {
    int PoolSize = ThreadV.Len();
    // Add empty tasks that will stop threads
    for (int ThreadN = 0; ThreadN < PoolSize; ThreadN++) {
        Execute(NULL, false);
    }

    // join waits for all threads to terminate after consuming NULL
    Notify->OnNotifyFmt(TNotifyType::ntInfo, "Waiting for %d threads to finish...", PoolSize);
    for (int ThreadN = 0; ThreadN < PoolSize; ThreadN++) {
        ThreadV[ThreadN].Join();
    }
    Notify->OnNotifyFmt(TNotifyType::ntInfo, "Destroyed %d threads...", PoolSize);
}

void TThreadPool::Execute(const TWPt<TRunnable>& Runnable, const bool& CheckNullP) {
    if (CheckNullP) { EAssertR(!Runnable.Empty(), "Runnable object should not be NULL!"); }
    Lock.Lock();
    OutstandingTasks++;
    TaskQ.Push(Runnable);
    Lock.Signal();

    uint64 QSize = TaskQ.Len();

    Lock.Release();

    if (QSize > 0 && QSize % 100 == 0) {
        Notify->OnNotifyFmt(TNotifyType::ntStat, "Task queue has %ld pending tasks!", QSize);
    }
}

int TThreadPool::GetOutstandingTasks() {
    Lock.Lock();
    int Res = OutstandingTasks;
    Lock.Release();
    return Res;
}

TWPt<TThreadPool::TRunnable> TThreadPool::WaitForTask(const uint64& ThreadId) {
    Lock.Lock();
    while (TaskQ.Empty()) {
        // releases lock and waits for signal
        // automatically acquires lock when it catches a signal
        Lock.WaitForSignal();
    }
    TWPt<TRunnable> Runnable = TaskQ.Pop();

    Lock.Release();

    return Runnable;
}
