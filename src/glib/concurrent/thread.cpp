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
// Thread executor
TThreadExecutor::TExecutorThread::TExecutorThread():
        TThread(),
        Executor(NULL),
        Runnable(NULL) { printf("created thread\n"); } //,
        //Notify(NULL) {}

TThreadExecutor::TExecutorThread::TExecutorThread(TThreadExecutor* _Executor): //, const PNotify& _Notify):
        TThread(),
        Executor(_Executor),
        Runnable(NULL) { printf("created thread for executor\n"); } //,
        //Notify(_Notify) {}


void TThreadExecutor::TExecutorThread::Run() {
    while (true) {
        try {
            // get a task
            printf("Waiting for work, %ld\n", GetThreadId());
            TWPt<TRunnable> Runnable = Executor->WaitForTask();
            // check if the executor was terminated
            if (Runnable.Empty()) { 
                printf("Thread got empty runnable, %ld\n", GetThreadId());
                break; }
            printf("Thread calling run, %ld\n", GetThreadId());
            // run the task
            Runnable->Run();
        } catch (const PExcept& Except) {
            printf("Exception in worker thread %ld: %s\n", GetThreadId(), Except->GetMsgStr().CStr());
        }
    }

    printf("Thread %ld exiting finished ...\n", GetThreadId());
}

TThreadExecutor::TThreadExecutor(const int& PoolSize): //, const PNotify& _Notify):
        ThreadV(),
        TaskQ(),
        Lock() { //,
        //Notify(_Notify) {

    printf("Creating executor with %d threads ...\n", PoolSize);

    for (int i = 0; i < PoolSize; i++) {
        ThreadV.Add(TExecutorThread(this)); //, Notify));
    }
    printf("Starting %d threads ...\n", PoolSize);
    ThreadV.StartAll();
}

TThreadExecutor::~TThreadExecutor() {
    printf("Destroying executor with %d threads ...\n", ThreadV.Len());
    // cancel the threads
    //Lock.Lock();

    for (int i = 0; i < ThreadV.Len(); i++) {
        if (ThreadV[i].IsAlive()) {
            printf("Canceling thread %ld...\n", ThreadV[i].GetThreadId());
            ThreadV[i].Cancel();
        }
    }

    //Lock.Release();
    // join
    printf("Calling join for %d...\n", ThreadV.Len());
    ThreadV.Join();
    printf("Destroyed %d...\n", ThreadV.Len());
}

void TThreadExecutor::Execute(const TWPt<TRunnable>& Runnable) {
    if (Runnable.Empty()) { return; }

    Lock.Lock();

    printf("Adding new runnable to queue ...\n");
    TaskQ.Push(Runnable);
    Lock.Signal();

    uint64 QSize = TaskQ.Len();

    Lock.Release();

    if (QSize > 0 && QSize % 100 == 0) {
        printf("Task queue has %ld pending tasks!\n", QSize);
    }
}

TWPt<TThreadExecutor::TRunnable> TThreadExecutor::WaitForTask() {
    Lock.Lock();
    printf("Waiting for task!!\n");
    while (TaskQ.Empty()) {
        Lock.WaitForSignal();
    }
    printf("Got for task!!\n");
    TWPt<TRunnable> Runnable = TaskQ.Pop();

    Lock.Release();

    return Runnable;
}
