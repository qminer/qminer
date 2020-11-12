/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/*
 * thread.h
 *
 *  Created on: May 24, 2010
 *      Author: tadej
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <base.h>
#include <pthread.h>

#define INFINITE TInt::Mx

class TBlocker {
protected:
    pthread_cond_t Event;
    pthread_mutex_t Mutex;

    //HANDLE Event;

public:
    TBlocker();
    ~TBlocker();

    void Block(int Msecs = TInt::Mx);
    void Release();
};

/**
 * Critical section - Allows only 1 thread to enter, but the same thread can enter multiple times
 */
class TCriticalSection {
protected:
    //CRITICAL_SECTION Cs;
    pthread_mutex_t Cs;
    pthread_mutexattr_t CsAttr;

public:
    TCriticalSection();
    ~TCriticalSection();

    void Enter();
    bool TryEnter();
    void Leave();
};

////////////////////////////////////////////
// Thread
ClassTP(TThread, PThread)// {
private:
    const static int STATUS_CREATED;
    const static int STATUS_STARTED;
    const static int STATUS_CANCELLED;
    const static int STATUS_FINISHED;

    pthread_t ThreadHandle;

    // Use for interrupting and waiting
    TBlocker* SleeperBlocker;
    TCriticalSection CriticalSection;

    volatile sig_atomic_t Status;

private:
    static void * EntryPoint(void * pArg);
    static void SetFinished(void *pArg);
public:
    TThread();

    TThread(const TThread& Other) {
        operator=(Other);
    }
    TThread& operator =(const TThread& Other);

    virtual ~TThread();

    // starts the thread
    void Start();
    // when started the thread calls this function
    virtual void Run() { printf("empty run\n"); };

    // terminates the thread
    void Cancel();

    // windows thread id
    uint64 GetThreadId() const { return (uint64)GetThreadHandle(); }
    // windows thread handle
    pthread_t GetThreadHandle() const { return ThreadHandle; }

    void Interrupt();
    void WaitForInterrupt(const int Msecs = INFINITE);

    int Join();

    bool IsAlive();
    bool IsCancelled();
    bool IsFinished();

    static int GetCoreCount();

};


// waits until the given thread finishes the execution
// - optionaly can be given a maximal number of miliseconds to wait
template <class TThreadType>
int ThreadWait(TThreadType& Thread, const int& MxMSecs = -1) {
    //DWORD dwMilliSeconds = (MxMSecs == -1) ? INFINITE : ((DWORD)MxMSecs);
    int dwMilliSeconds = (MxMSecs == -1) ? TInt::Mx : MxMSecs;
    const int Res = pthread_join(Thread.GetThreadHandle(), dwMilliSeconds);

            //WaitForSingleObject(Thread.GetThreadHandle(), dwMilliSeconds);
    return Res;//(Res == WAIT_ABANDONED) || (Res == WAIT_OBJECT_0);
}

// waits until at least one of threads from the given vector of threads
// finishes the execution
// - optionaly can be given a maximal number of miliseconds to wait
template <class TThreadType>
int ThreadWaitOne(TVec<TThreadType>& ThreadV, const int& MxMSecs = -1) {
    // prepare parameters
    /*const int Threads = ThreadV.Len();
    const int nCount = (int)Threads;
    const int dwMilliSeconds = (MxMSecs == -1) ? TInt::Mx : ((int)MxMSecs);
    // create array
    pthread_t* lpHandles = new pthread_t[Threads];
    for (int ThreadN = 0; ThreadN < Threads; ThreadN++) {
        lpHandles[ThreadN] = ThreadV[ThreadN].GetThreadHandle();
    }
    // wait
    //const int Res = pthread_
            //WaitForMultipleObjects(
        //nCount, lpHandles, waitAll, dwMilliSeconds);
    // delete array
    for (int ThreadN = 0; ThreadN)
    delete lpHandles;
    // check the output and finish
    if (Res == WAIT_TIMEOUT) { return -1; } // timed-out
    return (int)(Res - WAIT_OBJECT_0); // return the index of finished thread*/
    int ret = 0;
    for (int i = 0;i < ThreadV.Len(); i++) {
        ret = ThreadV[i].Join(MxMSecs);
    }
    return ret;
}


template<class TThreadType>
class TThreadV : public TVec<TThreadType> {
public:
    void StartAll() {
        for (int i = 0; i < this->Len(); i++) {
            (*this)[i].Start();
        }
    }
    int Join() {
        int Status = 0;
        for (int i = 0; i < this->Len(); i++) {
            Status = (*this)[i].Join();//ThreadWait<TThreadType>((*this)[i]);
        }
        return Status;
    }
};

class TCondVarLock;

////////////////////////////////////////////
// Mutex
class TMutex {
    friend class TCondVarLock;
private:
    TMutexType Type;
    pthread_mutex_t MutexHandle;
    pthread_mutexattr_t Attributes;


public:
    TMutex(const TMutexType& Type = TMutexType::mtFast, const bool& LockOnStartP = false);
    ~TMutex();

    // waits so the mutex is released and locks it
    bool Wait(const int& MxMSecs = -1);

    void GetLock();
    // releases the mutex
    bool Release();

    // mutex handle
    pthread_mutex_t GetThreadHandle() const { return MutexHandle; }
};

////////////////////////////////////////////
// Conditional variable lock
// blocks threads on a until a condition is fulfilled
class TCondVarLock {
private:
    pthread_cond_t CondVar;
    TMutex Mutex;

public:
    TCondVarLock();
    ~TCondVarLock();

    // locks the mutex
    void Lock();
    // releases the mutex
    bool Release();
    // must be locked before calling this method
    // unlocks the mutex and waits for a signal once it gets the signal the
    // mutex is automatically locked
    void WaitForSignal();
    // must be locked before signaling
    // unblocks at least one thread waiting on the
    // conditional variable
    void Signal();
    // must be locked before broadcasting
    // unblocks all threads waiting on the
    // conditional variable
    void Broadcast();
};


#endif /* THREAD_H_ */
